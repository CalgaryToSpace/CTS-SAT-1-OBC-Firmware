#include "unit_tests/unit_test_helpers.h"
#include "unit_tests/test_comms.h"

#include "comms_drivers/comms_tx.h"


uint8_t TEST_EXEC__validate_packet_sizes() {
    TEST_ASSERT(sizeof(COMMS_log_message_packet_t) == AX100_DOWNLINK_MAX_BYTES);
    TEST_ASSERT(sizeof(COMMS_tcmd_response_packet_t) == AX100_DOWNLINK_MAX_BYTES);
    TEST_ASSERT(sizeof(COMMS_bulk_file_downlink_packet_t) == AX100_DOWNLINK_MAX_BYTES);
    TEST_ASSERT(sizeof(COMMS_beacon_basic_packet_t) <= AX100_DOWNLINK_MAX_BYTES);

    return 0;
}
