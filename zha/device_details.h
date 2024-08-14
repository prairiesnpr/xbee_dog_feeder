#include <stdint.h>

#define NUM_ENDPOINTS 3

constexpr uint8_t one_zero_byte[] = {0x00};
constexpr uint8_t one_max_byte[] = {0xFF};
constexpr uint8_t two_zero_byte[] = {0x00, 0x00};
constexpr uint8_t four_zero_byte[] = {0x00, 0x00, 0x00, 0x00};
constexpr uint8_t bac_net_no_units[] = {0x5f, 0x00};
constexpr uint8_t max_feeding[] = {0x00, 0x00, 0x40, 0x41}; // 12
constexpr uint8_t min_feeding[] = {0x00, 0x00, 0x80, 0x3f}; // 1
constexpr uint8_t default_feeding[] = {0x00, 0x00, 0x00, 0x40}; // 2
constexpr uint8_t max_feed_count[] = {0x00, 0x00, 0x7f, 0x43}; // 255


constexpr uint8_t level_states[] = {0x03, 0x00};
constexpr uint8_t feed_result_states[] = {0x03, 0x00};
constexpr uint8_t last_feed_states[] = {0x02, 0x00};

// Reuse these to save SRAM
constexpr char manufacturer[]  = "iSilentLLC";
constexpr char feeder_model[] = "Dog Feeder";
constexpr char jam_desc[] =  "Jammed";
constexpr char motor_state_desc[] = "Motor State";
constexpr char feeder_level_desc[] = "Food Level";
constexpr char feed_result_desc[] = "Last Feed Result";
constexpr char feed_count_desc[] = "Feed Count";
constexpr char last_feed_trigger_desc[] = "Last Trigger";


constexpr uint8_t feeder_result_states[] = {ZCL_CHAR_STR, 0x03, 0x00, 0x02, 0x4F, 0x4B, 0x07, 0x50, 0x61, 0x72, 0x74, 0x69, 0x61, 0x6C, 0x04, 0x46, 0x61, 0x69, 0x6C};
constexpr uint8_t feeder_level_states[] = {ZCL_CHAR_STR, 0x03, 0x00, 0x04, 0x46, 0x75, 0x6C, 0x6C, 0x03, 0x4C, 0x6F, 0x77, 0x05, 0x45, 0x6D, 0x70, 0x74, 0x79};
constexpr uint8_t last_feed_trig_states[] = {ZCL_CHAR_STR, 0x02, 0x00, 0x05, 0x4C, 0x6F, 0x63, 0x61, 0x6C, 0x06, 0x52, 0x65, 0x6D, 0x6F, 0x74, 0x65};

//constexpr char *feeder_level_states[] = {"Full", "Low", "Empty"};
//constexpr char *feed_result_states[] = {"OK", "Partial", "Fail"};

/*

void BuildArray(char** tarray,  uint8_t* sarray, uint16_t elements){
  uint8_t bufferlen = 2;

  for (uint8_t i = 0; i<elements; i++) {
    bufferlen += (sarray[i] + 1);

  }
  uint8_t bufpos = 0;
  uint8_t buffer[bufferlen];

  memcpy(buffer, &elements, 2);
  bufpos = 2;
  for (uint8_t i = 0; i<elements; i++) {
    memcpy(buffer + bufpos + i, &sarray[i], 1);
    bufpos += 1;
    memcpy(buffer + bufpos, tarray[i], sarray[i]);
    bufpos += sarray[i];

  }

  for (uint8_t i=0; i<bufferlen;i++){
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

*/

attribute BuildStringAtt(uint16_t a_id, char *value, uint8_t size, uint8_t a_type)
{
    uint8_t *value_t = (uint8_t *)value;
    return attribute(a_id, value_t, size, a_type, 0x01);
}

attribute manuf_attr = BuildStringAtt(MANUFACTURER_ATTR, const_cast<char *>(manufacturer), sizeof(manufacturer), ZCL_CHAR_STR);
attribute feeder_model_attr = BuildStringAtt(MODEL_ATTR, const_cast<char *>(feeder_model), sizeof(feeder_model), ZCL_CHAR_STR);
attribute feeder_jammed_desc_attr = BuildStringAtt(DESCRIPTION_ATTR, const_cast<char *>(jam_desc), sizeof(jam_desc), ZCL_CHAR_STR);
attribute feeder_level_desc_attr = BuildStringAtt(DESCRIPTION_ATTR, const_cast<char *>(feeder_level_desc), sizeof(feeder_level_desc), ZCL_CHAR_STR);
attribute feed_result_desc_attr = BuildStringAtt(DESCRIPTION_ATTR, const_cast<char *>(feed_result_desc), sizeof(feed_result_desc), ZCL_CHAR_STR);
attribute motor_state_desc_attr = BuildStringAtt(DESCRIPTION_ATTR, const_cast<char *>(motor_state_desc), sizeof(motor_state_desc), ZCL_CHAR_STR);
attribute feed_count_desc_attr = BuildStringAtt(DESCRIPTION_ATTR, const_cast<char *>(feed_count_desc), sizeof(feed_count_desc), ZCL_CHAR_STR);
attribute last_feed_trig_desc_attr = BuildStringAtt(DESCRIPTION_ATTR, const_cast<char *>(last_feed_trigger_desc), sizeof(last_feed_trigger_desc), ZCL_CHAR_STR);

attribute eng_unit_none_attr = attribute(ENG_UNITS_ATTR, const_cast<uint8_t *>(bac_net_no_units), sizeof(bac_net_no_units), ZCL_ENUM16, 0x01);
attribute binary_status_flag = attribute(BINARY_STATUS_FLG, const_cast<uint8_t *>(one_zero_byte), sizeof(one_zero_byte), ZCL_MAP8, 0x01); // Status flags
attribute out_of_service = attribute(OUT_OF_SERVICE, const_cast<uint8_t *>(one_zero_byte), sizeof(one_zero_byte), ZCL_BOOL, 0x01); // Status flags
attribute max_feedings_attr = attribute(MAX_PV_ATTR, const_cast<uint8_t *>(max_feeding), sizeof(max_feeding), ZCL_SINGLE, 0x01); // 12
attribute min_feedings_attr = attribute(MIN_PV_ATTR, const_cast<uint8_t *>(min_feeding), sizeof(min_feeding), ZCL_SINGLE, 0x01); // 0

attribute feeder_result_states_attr = attribute(STATE_TEXT_ATTR, const_cast<uint8_t *>(feeder_result_states), sizeof(feeder_result_states), ZCL_ARRAY, 0x01);
attribute feeder_level_states_attr = attribute(STATE_TEXT_ATTR, const_cast<uint8_t *>(feeder_level_states), sizeof(feeder_level_states), ZCL_ARRAY, 0x01);
attribute last_feed_trig_states_attr = attribute(STATE_TEXT_ATTR, const_cast<uint8_t *>(last_feed_trig_states), sizeof(last_feed_trig_states), ZCL_ARRAY, 0x01);

attribute feeder_basic_attr[]{
    manuf_attr,
    feeder_model_attr};

attribute feeder_jammed_attr[]{
    {BINARY_PV_ATTR, const_cast<uint8_t *>(one_zero_byte), 1, ZCL_BOOL},  // present value
    binary_status_flag,
    out_of_service,
    feeder_jammed_desc_attr 
};

attribute motor_state_attr[]{
    {BINARY_PV_ATTR, const_cast<uint8_t *>(one_zero_byte), 1, ZCL_BOOL},  // present value
    binary_status_flag,
    out_of_service,
    motor_state_desc_attr 
};
attribute feeder_level_attr[] {
 {BINARY_PV_ATTR, const_cast<uint8_t *>(two_zero_byte), 2, ZCL_UINT16_T}, //not Binary
 feeder_level_desc_attr,
 {NUM_OF_STATES, const_cast<uint8_t *>(level_states), 2, ZCL_UINT16_T},
 out_of_service,
 binary_status_flag,
 feeder_level_states_attr
};

attribute last_feed_trigger_attr[] {
 {BINARY_PV_ATTR, const_cast<uint8_t *>(two_zero_byte), 2, ZCL_UINT16_T}, //not Binary
 last_feed_trig_desc_attr,
 {NUM_OF_STATES, const_cast<uint8_t *>(last_feed_states), 2, ZCL_UINT16_T},
 out_of_service,
 binary_status_flag,
 last_feed_trig_states_attr
};
attribute feed_result_attr[] {
 {BINARY_PV_ATTR, const_cast<uint8_t *>(two_zero_byte), 2, ZCL_UINT16_T}, //not Binary
 feed_result_desc_attr,
 {NUM_OF_STATES, const_cast<uint8_t *>(feed_result_states), 2, ZCL_UINT16_T},
 out_of_service,
 binary_status_flag,
 feeder_result_states_attr
};


attribute feeder_feedings_attr[] {
    eng_unit_none_attr,
    max_feedings_attr,
    min_feedings_attr,
    {BINARY_PV_ATTR, const_cast<uint8_t *>(default_feeding), 4, ZCL_SINGLE}, // present value
    out_of_service,
};

attribute feeder_switch_attr[] {
    {CURRENT_STATE, const_cast<uint8_t *>(one_zero_byte), 1, ZCL_BOOL},
};

attribute feed_count_analog_in_attr[] = {
    {BINARY_PV_ATTR, const_cast<uint8_t *>(four_zero_byte), 4, ZCL_SINGLE}, // present value
    binary_status_flag,
    out_of_service,
    eng_unit_none_attr,
    feed_count_desc_attr,
    {MAX_PV_ATTR, const_cast<uint8_t *>(max_feed_count), sizeof(max_feed_count), ZCL_SINGLE, 0x01},
    {MIN_PV_ATTR, const_cast<uint8_t *>(four_zero_byte), sizeof(four_zero_byte), ZCL_SINGLE, 0x01}

};

Cluster feeder_in_clusters[] = {
    Cluster(BASIC_CLUSTER_ID, feeder_basic_attr, sizeof(feeder_basic_attr) / sizeof(*feeder_basic_attr)),
    Cluster(BINARY_INPUT_CLUSTER_ID, feeder_jammed_attr, sizeof(feeder_jammed_attr) / sizeof(*feeder_jammed_attr)),
    Cluster(MULTISTATE_IN_CLUSTER_ID, feeder_level_attr, sizeof(feeder_level_attr) / sizeof(*feeder_level_attr)),
    Cluster(ANALOG_IN_CLUSTER_ID, feed_count_analog_in_attr, sizeof(feed_count_analog_in_attr) / sizeof(*feed_count_analog_in_attr)),
    Cluster(ON_OFF_CLUSTER_ID, feeder_switch_attr, sizeof(feeder_switch_attr) / sizeof(*feeder_switch_attr)),
    Cluster(ANALOG_OUT_CLUSTER_ID, feeder_feedings_attr, sizeof(feeder_feedings_attr) / sizeof(*feeder_feedings_attr))
    };

Cluster feeder_diag_in_clusters[] = {
    Cluster(MULTISTATE_IN_CLUSTER_ID, feed_result_attr, sizeof(feed_result_attr) / sizeof(*feed_result_attr)),
    Cluster(BINARY_INPUT_CLUSTER_ID, motor_state_attr, sizeof(motor_state_attr) / sizeof(*motor_state_attr))
    };

Cluster last_feeder_trigger_in_clusters[] = {
    Cluster(MULTISTATE_IN_CLUSTER_ID, last_feed_trigger_attr, sizeof(last_feed_trigger_attr) / sizeof(*last_feed_trigger_attr))
};

Cluster out_clusters[] = {};
Endpoint ENDPOINTS[NUM_ENDPOINTS] = {
    Endpoint(1, ON_OFF_OUTPUT, feeder_in_clusters, out_clusters, sizeof(feeder_in_clusters) / sizeof(*feeder_in_clusters), 0),
    Endpoint(2, ON_OFF_SENSOR, feeder_diag_in_clusters, out_clusters, sizeof(feeder_diag_in_clusters) / sizeof(*feeder_diag_in_clusters), 0),
    Endpoint(3, COMBINED_INTERFACE, last_feeder_trigger_in_clusters, out_clusters, sizeof(last_feeder_trigger_in_clusters) / sizeof(*last_feeder_trigger_in_clusters), 0)
};