#include <stdint.h>

#define NUM_ENDPOINTS 2

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

// Reuse these to save SRAM
constexpr char manufacturer[]  = "iSilentLLC";
constexpr char feeder_model[] = "Dog Feeder";
constexpr char jam_desc[] =  "Jammed";
constexpr char motor_state_desc[] = "Motor State";
constexpr char feeder_level_desc[] = "Food Level";
constexpr char feed_result_desc[] = "Last Feed Result";
constexpr char feed_count_desc[] = "Feed Count";


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


attribute eng_unit_none_attr = attribute(ENG_UNITS_ATTR, const_cast<uint8_t *>(bac_net_no_units), sizeof(bac_net_no_units), ZCL_ENUM16, 0x01);
attribute binary_status_flag = attribute(BINARY_STATUS_FLG, const_cast<uint8_t *>(one_zero_byte), sizeof(one_zero_byte), ZCL_MAP8, 0x01); // Status flags
attribute out_of_service = attribute(OUT_OF_SERVICE, const_cast<uint8_t *>(one_zero_byte), sizeof(one_zero_byte), ZCL_BOOL, 0x01); // Status flags
attribute max_feedings_attr = attribute(MAX_PV_ATTR, const_cast<uint8_t *>(max_feeding), sizeof(max_feeding), ZCL_SINGLE, 0x01); // 12
attribute min_feedings_attr = attribute(MIN_PV_ATTR, const_cast<uint8_t *>(min_feeding), sizeof(min_feeding), ZCL_SINGLE, 0x01); // 0

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
 binary_status_flag
};

attribute feed_result_attr[] {
 {BINARY_PV_ATTR, const_cast<uint8_t *>(two_zero_byte), 2, ZCL_UINT16_T}, //not Binary
 feed_result_desc_attr,
 {NUM_OF_STATES, const_cast<uint8_t *>(feed_result_states), 2, ZCL_UINT16_T},
 out_of_service,
 binary_status_flag
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

Cluster out_clusters[] = {};
Endpoint ENDPOINTS[NUM_ENDPOINTS] = {
    Endpoint(1, ON_OFF_OUTPUT, feeder_in_clusters, out_clusters, sizeof(feeder_in_clusters) / sizeof(*feeder_in_clusters), 0),
    Endpoint(2, ON_OFF_SENSOR, feeder_diag_in_clusters, out_clusters, sizeof(feeder_diag_in_clusters) / sizeof(*feeder_diag_in_clusters), 0)
};
