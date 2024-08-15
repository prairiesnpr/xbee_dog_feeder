#include <arduino-timer.h>
#include <xbee_zha.h>
#include "zha/device_details.h"
#include "dog_feeder_functions.h"
#include <SoftwareSerial.h>

#define XBEE_RST 3

#define feederRX 9
#define feederTX 10

#define xbeeTX 1
#define xbeeRX 0

#define BASIC_ENDPOINT 0
#define FEEDER_ENDPOINT 1
#define DIAG_ENDPOINT 2
#define FEED_TRIG_ENDPOINT 3

#define FEED_EEPROM_LOC 8

#define START_LOOPS 100

uint8_t start_fails = 0;
uint8_t init_status_sent = 0;

void (*resetFunc)(void) = 0;

auto timer = timer_create_default(); // create a timer with default settings

unsigned long loop_time = millis();
unsigned long last_msg_time = loop_time - 1000;

FeederState feed_state;

void setup()
{
  pinMode(XBEE_RST, OUTPUT);

  // Reset xbee
  digitalWrite(XBEE_RST, LOW);
  delay(5);
  digitalWrite(XBEE_RST, HIGH);

  Serial.begin(115200);
  Serial2.begin(9600);
  feed_state.start(Serial2, 0x01);
  Serial.println(F("Startup s1"));
  Serial1.begin(9600);
  zha.Start(Serial1, zhaClstrCmd, zhaWriteAttr, NUM_ENDPOINTS, ENDPOINTS);

  // Set up callbacks, shouldn't have to do this here, but bad design...
  zha.registerCallbacks(atCmdResp, zbTxStatusResp, otherResp, zdoReceive);

  Serial.println(F("CB Conf"));

  timer.every(60000, update_sensors);
}

void update_feed_count(bool force = 0x00)
{
  Endpoint end_point = zha.GetEndpoint(FEEDER_ENDPOINT);
  Cluster clstr = end_point.GetCluster(ANALOG_IN_CLUSTER_ID);

  attribute *pv_attr;
  uint8_t attr_exists = clstr.GetAttr(&pv_attr, BINARY_PV_ATTR);
  if (pv_attr->GetFloatValue() != feed_state.feed_count || force)
  {
    Serial.print(F("cnt was: "));
    Serial.print(pv_attr->GetFloatValue());
    Serial.print(F(", Now: "));
    Serial.println(feed_state.feed_count);
    pv_attr->SetFloatValue((float)feed_state.feed_count);
    zha.sendAttributeRpt(clstr.id, pv_attr, end_point.id, 1);
  }
}

void update_feed_result(bool force = 0x00)
{
  Endpoint end_point = zha.GetEndpoint(DIAG_ENDPOINT);
  Cluster clstr = end_point.GetCluster(MULTISTATE_IN_CLUSTER_ID);
  attribute *pv_attr;
  uint8_t attr_exists = clstr.GetAttr(&pv_attr, BINARY_PV_ATTR);
  if (pv_attr->GetIntValue(0x00) != feed_state.last_feed_result || force)
  {
    pv_attr->SetValue(feed_state.last_feed_result);
    zha.sendAttributeRpt(clstr.id, pv_attr, end_point.id, 1);
  }
}

void update_motor_state(bool force = 0x00)
{
  Endpoint end_point = zha.GetEndpoint(DIAG_ENDPOINT);
  Cluster clstr = end_point.GetCluster(BINARY_INPUT_CLUSTER_ID);
  attribute *pv_attr;
  uint8_t attr_exists = clstr.GetAttr(&pv_attr, BINARY_PV_ATTR);
  if (pv_attr->GetIntValue(0x00) != feed_state.motor_state || force)
  {
    pv_attr->SetValue(feed_state.motor_state);
    zha.sendAttributeRpt(clstr.id, pv_attr, end_point.id, 1);
  }
}

void update_feed_level(bool force = 0x00)
{
  Endpoint end_point = zha.GetEndpoint(FEEDER_ENDPOINT);
  Cluster clstr = end_point.GetCluster(MULTISTATE_IN_CLUSTER_ID);
  attribute *pv_attr;
  uint8_t attr_exists = clstr.GetAttr(&pv_attr, BINARY_PV_ATTR);
  if (pv_attr->GetIntValue(0x00) != feed_state.food_level || force)
  {
    pv_attr->SetValue(feed_state.food_level);
    zha.sendAttributeRpt(clstr.id, pv_attr, end_point.id, 1);
  }
}

void update_jam_sensor(bool force = 0x00)
{
  Endpoint end_point = zha.GetEndpoint(FEEDER_ENDPOINT);
  Cluster clstr = end_point.GetCluster(BINARY_INPUT_CLUSTER_ID);
  attribute *pv_attr;
  uint8_t attr_exists = clstr.GetAttr(&pv_attr, BINARY_PV_ATTR);
  if (pv_attr->GetIntValue(0x00) != feed_state.is_jammed || force)
  {
    pv_attr->SetValue(feed_state.is_jammed);
    zha.sendAttributeRpt(clstr.id, pv_attr, end_point.id, 1);
  }
}

void update_feed_setting()
{
  Endpoint end_point = zha.GetEndpoint(FEEDER_ENDPOINT);
  Cluster analog_out_cluster = end_point.GetCluster(ANALOG_OUT_CLUSTER_ID);
  attribute *analog_out_attr;
  uint8_t attr_exists = analog_out_cluster.GetAttr(&analog_out_attr, BINARY_PV_ATTR);

  // Need to load the settings from EEPROM and save to the Analog Attr
  uint32_t saved_value = ReadInt(FEED_EEPROM_LOC);
  analog_out_attr->SetValue(saved_value);
  zha.sendAttributeRpt(analog_out_cluster.id, analog_out_attr, end_point.id, 1);
}
void update_last_feed_trigger(bool force = 0x00)
{
  Endpoint end_point = zha.GetEndpoint(FEED_TRIG_ENDPOINT);
  Cluster clstr = end_point.GetCluster(MULTISTATE_IN_CLUSTER_ID);
  attribute *pv_attr;
  uint8_t attr_exists = clstr.GetAttr(&pv_attr, BINARY_PV_ATTR);
  if (pv_attr->GetIntValue(0x00) != feed_state.last_feed_source || force)
  {
    pv_attr->SetValue(feed_state.last_feed_source);
    zha.sendAttributeRpt(clstr.id, pv_attr, end_point.id, 1);
  }
}
bool update_sensors(void *)
{
  // update_jam_sensor(0x01);
  update_feed_level(0x01);
  return true;
}

void SaveInt(uint32_t value, uint8_t start_address)
{
  EEPROM.write(start_address, (value >> 24) & 0xFF);
  EEPROM.write(start_address + 1, (value >> 16) & 0xFF);
  EEPROM.write(start_address + 2, (value >> 8) & 0xFF);
  EEPROM.write(start_address + 3, value & 0xFF);
}

uint32_t ReadInt(uint8_t address)
{
  return ((uint32_t)EEPROM.read(address) << 24) +
         ((uint32_t)EEPROM.read(address + 1) << 16) +
         ((uint32_t)EEPROM.read(address + 2) << 8) +
         (uint32_t)EEPROM.read(address + 3);
}

void send_inital_state()
{

  update_jam_sensor();
  update_feed_level();
  update_feed_count();
  update_feed_result();
  update_motor_state();
  update_feed_setting();
  update_last_feed_trigger();
}

void loop()
{
  zha.loop();

  if (zha.dev_status == READY)
  {
    if (init_status_sent)
    {
      if ((loop_time - last_msg_time) > 1000)
      {
        last_msg_time = millis();
      }
      update_jam_sensor();
      update_feed_level();
      update_feed_count();
      update_feed_result();
      update_motor_state();
      update_last_feed_trigger();
    }

    if (!init_status_sent)
    {
      Serial.println(F("Snd Init States"));
      init_status_sent = 1;
      send_inital_state();
    }
  }
  else if ((loop_time - last_msg_time) > 1000)
  {
    Serial.print(F("Not Started "));
    Serial.print(start_fails);
    Serial.print(F(" of "));
    Serial.println(START_LOOPS);

    last_msg_time = millis();
    if (start_fails > 15)
    {
      // Sometimes we don't get a response from dev ann, try a transmit and see if we are good
      send_inital_state();
    }
    if (start_fails > START_LOOPS)
    {
      resetFunc();
    }
    start_fails++;
  }
  feed_state.loop();
  timer.tick();
  loop_time = millis();
}

void zhaWriteAttr(ZBExplicitRxResponse &erx)
{

  Serial.println(F("Write Cmd"));
  if (erx.getClusterId() == ANALOG_OUT_CLUSTER_ID)
  {
    Serial.println(F("A Out"));
    Endpoint end_point = zha.GetEndpoint(erx.getDstEndpoint());
    // Cluster Command, so it's a write command
    // uint8_t len_data = erx.getDataLength() - 3;
    // uint16_t attr_rqst[len_data / 2];
    uint32_t a_val_i;
    a_val_i = ((uint32_t)erx.getFrameData()[erx.getDataOffset() + erx.getDataLength() - 1] << 24) |
              ((uint32_t)erx.getFrameData()[erx.getDataOffset() + erx.getDataLength() - 2] << 16) |
              ((uint32_t)erx.getFrameData()[erx.getDataOffset() + erx.getDataLength() - 3] << 8) |
              ((uint32_t)erx.getFrameData()[erx.getDataOffset() + erx.getDataLength() - 4]);

    Serial.print(F("Flt: "));
    float a_val;
    memcpy(&a_val, &a_val_i, 4);
    Serial.println(a_val, 4);

    Cluster ai_clstr = end_point.GetCluster(erx.getClusterId());
    attribute *ai_attr;
    uint8_t attr_exists = ai_clstr.GetAttr(&ai_attr, BINARY_PV_ATTR);
    pv_attr->SetFloatValue(a_val);

    Serial.print(F("Flt now: "));
    Serial.println(pv_attr->GetFloatValue(), 2);
    zha.sendAttributeWriteResp(ai_clstr.id, pv_attr, end_point.id, 1, 0x01, erx.getFrameData()[erx.getDataOffset() + 1]);

    uint8_t mem_loc;
    if (end_point.id == FEEDER_ENDPOINT)
    {
      Serial.println(F("FEED"));
      mem_loc = FEED_EEPROM_LOC;
    }
    if (ReadInt(mem_loc) != a_val_i)
    {
      Serial.println(F("Sv Val"));
      SaveInt(a_val_i, mem_loc);
    }
  }
}

void SetAttr(uint8_t ep_id, uint16_t cluster_id, uint16_t attr_id, uint8_t value, uint8_t rqst_seq_id)
{
  Endpoint end_point = zha.GetEndpoint(ep_id);
  Cluster cluster = end_point.GetCluster(cluster_id);
  attribute *attr;
  uint8_t attr_exists = cluster.GetAttr(&attr, attr_id);

  Serial.print("Clstr: ");
  Serial.println(cluster_id, HEX);

  if (cluster_id == ON_OFF_CLUSTER_ID)
  {
    // We don't want to set value here, value is set by the door opening or closing
    if (value == 0x00 || 0x01)
    {
      Serial.print(F("Toggle: "));
      Serial.println(end_point.id);
      // We never mess with the attribute, since we this is just a toggle
      zha.sendAttributeCmdRsp(cluster_id, attr, ep_id, 1, value, zha.cmd_seq_id); // Tell sender that we did what we were told to
      Cluster feed_qty_cluster = end_point.GetCluster(ANALOG_OUT_CLUSTER_ID);
      attribute *feed_qty_attr;
      uint8_t attr_exists = feed_qty_cluster.GetAttr(&feed_qty_attr, BINARY_PV_ATTR);
      uint8_t feed_qty = (uint8_t)feed_qty_attr->GetFloatValue();
      feed_state.genFeed(feed_qty);
      delay(100);
      zha.sendAttributeRpt(cluster.id, attr, end_point.id, 1);
    }
  }
}

void zhaClstrCmd(ZBExplicitRxResponse &erx)
{
  Serial.println(F("Clstr Cmd"));
  if (erx.getDstEndpoint() == FEEDER_ENDPOINT)
  {
    Serial.println(F("Feeder Ep"));
    if (erx.getClusterId() == ON_OFF_CLUSTER_ID)
    {
      Serial.println(F("ON/OFF"));
      uint8_t new_state = erx.getFrameData()[erx.getDataOffset() + 2];

      if (new_state == 0x00)
      {
        Serial.println(F("Off"));
        SetAttr(erx.getDstEndpoint(), erx.getClusterId(), CURRENT_STATE, new_state, erx.getFrameData()[erx.getDataOffset() + 1]);
      }
      else if (new_state == 0x01)
      {
        Serial.println(F("On"));
        SetAttr(erx.getDstEndpoint(), erx.getClusterId(), CURRENT_STATE, new_state, erx.getFrameData()[erx.getDataOffset() + 1]);
      }
      else
      {
        Serial.print(F("Inv State: "));
        Serial.println(new_state, HEX);
      }
    }
  }

  if (erx.getClusterId() == BASIC_CLUSTER_ID)
  {
    Serial.println(F("Basic Clstr"));
  }
}
