#define TUYA_HB 0x00
#define TUYA_STATE 0x07
#define TUYA_DATE 0x1c

#define FEEDER_FULL 0x00
#define FEEDER_LOW 0x01
#define FEEDER_EMPTY 0x02

#define FEED_OK 0x00
#define FEED_PART 0x01
#define FEED_FAIL 0x02

#define FEED_MOTOR_OK 0x01   // Full required rotations
#define FEED_MOTOR_FAIL 0x81 // Didn't trigger the microswitch the same number as the portion size

#define FEED_STATE_ID 0x66
#define FEED_RSLT_ID 0x6A
#define FEED_PORTION_ID 0x65
#define UKN_STATE 0x68 // Need to figure out what this is

constexpr uint8_t bufferlen = 18;
constexpr uint8_t pkt_start_id[] = {0x55, 0xaa};

class FeederState
{
public:
    uint8_t food_level;
    bool is_jammed;
    uint8_t last_feed_result;
    uint8_t motor_state;
    uint8_t feed_count;
    bool debug;
    void start(Stream &dogSerial, bool debug = 0x00)
    {
        this->food_level = FEEDER_EMPTY;
        this->is_jammed = 0x00;
        this->last_feed_result = FEED_OK;
        this->motor_state = FEED_MOTOR_OK;
        this->feed_count = 0x00;
        this->dogSerial = &dogSerial;
        this->debug = debug;
    }
    void loop()
    {
        this->listenForData();
        this->recvDogPacket();
    }
    void genFeed(uint8_t feed_port)
    {
        uint8_t cmd[] = {0x55, 0xAA, 0x00, 0x06, 0x00, 0x08, 0x65, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, feed_port, 0x00};
        cmd[14] = calculate_chk_sum(cmd, 14);
        this->dogSerial->write(cmd, 15);
    }

private:
    uint8_t buffer[bufferlen];
    uint8_t pktstart;
    uint8_t pktstop;
    uint8_t bufpos;
    bool new_data = 0x00;
    bool pkt_in_progress = 0x00;
    uint8_t pktpos = 0x00;
    uint16_t pktlen;
    Stream *dogSerial;
    uint8_t calculate_chk_sum(uint8_t *pkt, uint8_t bufend)
    {
        // Start from the header, add up all the bytes, and then divide the sum by 256 to get the remainder.
        uint32_t bsum = 0;
        for (uint8_t i = 0; i < bufend; i++)
        {
            bsum += pkt[i];
        }
        return bsum % 256;
    }
    void parse_state_pkt(uint8_t *buffer, uint8_t *dpid, uint8_t *dtype, uint32_t *dval)
    {
        *dpid = buffer[6];
        *dtype = buffer[7];
        uint16_t dlen;
        memcpy(&dlen, buffer + 8, 2);
        dlen = SWAP_UINT16(dlen);
        memcpy(dval, buffer + 10, dlen);
        *dval = SWAP_UINT32(*dval);
        if (debug)
        {
            Serial.print(F("State, ID: "));
            Serial.print(*dpid, HEX);
            Serial.print(F(", Typ: "));
            Serial.print(*dtype, HEX);
            Serial.print(", ValLen: ");
            Serial.print(dlen);
            Serial.print(", Val: ");
            Serial.println(*dval, HEX);
        }
    }

    void feed_state_handler(uint32_t *dval)
    {
        this->food_level = (*dval >> 24) & 0xFF;
        if (this->debug)
        {
            Serial.print(this->food_level, HEX);
            Serial.print(F(": FEEDER "));
            if (this->food_level == FEEDER_EMPTY)
            {
                Serial.println(F("EMPTY"));
            }
            if (this->food_level == FEEDER_FULL)
            {
                Serial.println(F("FULL"));
            }
            if (this->food_level == FEEDER_LOW)
            {
                Serial.println(F("LOW"));
            }
        }
    }

    void feed_result_handler(uint32_t *dval)
    {
        this->last_feed_result = (*dval >> 24) & 0xFF;
        uint8_t byte1 = (*dval >> 16) & 0xFF;
        uint8_t motor_state = (*dval >> 8) & 0xFF;
        this->feed_count = (*dval >> 0) & 0xFF; // May be auto also, need to test, just know it increments by 1
        if (this->debug)
        {
            Serial.print(this->last_feed_result, HEX);
            Serial.print(F(": FEED RSLT "));
            if (this->last_feed_result == FEED_FAIL)
            {
                Serial.println(F("FAIL"));
            }
            if (this->last_feed_result == FEED_PART)
            {
                Serial.println(F("PART"));
            }
            if (this->last_feed_result == FEED_OK)
            {
                Serial.println(F("OK"));
            }

            Serial.print(motor_state, HEX);
            Serial.print(F(": FEED MTR ST "));
            if (this->motor_state == FEED_MOTOR_FAIL)
            {
                Serial.println(F("MOTOR FAIL"));
                this->motor_state = 0x01;
            }
            else if (this->motor_state == FEED_MOTOR_OK)
            {
                Serial.println(F("MOTOR OK"));
                this->motor_state = 0x00;
            }
            else
            {
                Serial.println(F("MOTOR UKN"));
                this->motor_state = 0x01;
            }

            Serial.print(F("UKN 2nd B: "));
            Serial.println(byte1, HEX);
            Serial.print(F("Feed Ct: "));
            Serial.println(this->feed_count);
        }
    }

    void state_handler(uint8_t *dpid, uint8_t *dtype, uint32_t *dval)
    {
        if (*dpid == FEED_STATE_ID)
        {
            feed_state_handler(dval);
        }
        else if (*dpid == FEED_RSLT_ID)
        {
            feed_result_handler(dval);
        }
        else if (*dpid == FEED_PORTION_ID)
        {
            Serial.println(F("FEED PORT NOT IMP"));
        }
        else
        {
            Serial.print(F("State Not Imp: "));
            Serial.println(*dpid);
        }
    }

    void recvDogPacket()
    {
        if (new_data)
        {
            for (uint8_t i = 0; i < pktstop; i++)
            {
                Serial.print(buffer[i], HEX);
            }
            if (pktpos)
            {
                Serial.println();
            }

            if (buffer[3] == TUYA_STATE)
            {
                uint8_t dpid;
                uint8_t dtype;
                uint32_t dval;
                parse_state_pkt(buffer, &dpid, &dtype, &dval);
                state_handler(&dpid, &dtype, &dval);
            }
            pktpos = 0;
            pktstop = 0;
            pktstart = 0;
            new_data = 0;
            pktlen = 0;
            memset(buffer, 0, bufferlen);
        }
    }

    void listenForData()
    {

        while (this->dogSerial->available() && !new_data)
        {
            buffer[pktpos] = Serial2.read();
            if (pktpos == 1)
            {
                if (buffer[0] == pkt_start_id[0] &&
                    buffer[1] == pkt_start_id[1])
                {
                    // Serial.println(F("Pkt started"));
                }
                else
                {
                    pktpos = 0;
                    pktstart = 0;
                    pktstop = 0;
                    new_data = 0;
                    pktlen = 0;
                    buffer[0] = buffer[1];
                    memset(buffer + 1, 0x00, bufferlen - 1);
                    return;
                }
            }
            else if (pktpos == 3)
            {
                /*
                Serial.print(F("Pkt Type: "));
                if (buffer[pktpos] == TUYA_DATE)
                {
                    Serial.println(F("DT"));
                }
                else if (buffer[pktpos] == TUYA_STATE)
                {
                    Serial.println(F("ST"));
                }
                else if (buffer[pktpos] == TUYA_HB)
                {
                    Serial.println(F("HB"));
                }
                else
                {
                    Serial.println(buffer[pktpos]);
                }
                */
            }
            else if (pktpos == 5)
            {
                memcpy(&pktlen, buffer + pktpos - 1, 2);
                pktlen = SWAP_UINT16(pktlen);
            }
            else if (pktlen && pktpos == pktlen + 6)
            {
                pktstop = pktlen + 6;
                if (calculate_chk_sum(buffer, pktstop) == buffer[pktpos])
                {
                    new_data = 0x01;
                }
                else
                {
                    Serial.print(F("BD CHKSUM: "));
                    for (uint8_t k = 0; k < (pktlen + 7); k++)
                    {
                        Serial.print(buffer[k], HEX);
                        Serial.print(" ");
                    }
                    Serial.println();
                    Serial.print(F("Buf Pos: "));
                    Serial.print(pktpos);
                    Serial.print(F("Exp: "));
                    Serial.println(buffer[pktpos], HEX);
                    pktpos = 0;
                    pktstart = 0;
                    pktstop = 0;
                    new_data = 0;
                    pktlen = 0;
                    buffer[0] = buffer[1];
                    memset(buffer + 1, 0x00, bufferlen - 1);
                    return;
                }
            }
            pktpos++;
            if (pktpos >= bufferlen)
            {
                Serial.println(F("Buf Overflow"));
                pktpos = 0;
                pktstart = 0;
                pktstop = 0;
                new_data = 0;
                buffer[0] = buffer[1];
                memset(buffer + 1, 0x00, bufferlen - 1);
                return;
            }
        }
    }
};
