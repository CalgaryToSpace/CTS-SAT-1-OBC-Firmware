#include "unit_tests/test_eps_struct_packers.h"

#include "eps_drivers/eps_types.h"

#include "unit_tests/unit_test_helpers.h"

uint8_t TEST_EXEC__EPS_check_type_sizes() {
    // Check that all types have the same size
    TEST_ASSERT(sizeof(EPS_vpid_eng_t) == 6);
    TEST_ASSERT(sizeof(EPS_battery_pack_datatype_eng_t) == 22);
    TEST_ASSERT(sizeof(EPS_conditioning_channel_datatype_eng_t) == 14);
    TEST_ASSERT(sizeof(EPS_conditioning_channel_short_datatype_eng_t) == 8);
    TEST_ASSERT(sizeof(EPS_struct_system_status_t) == (36-5));
    TEST_ASSERT(sizeof(EPS_struct_pdu_overcurrent_fault_state_t) == (78-6));
    // EPS_struct_pbu_abf_placed_state_t is 2 enums, 32-bits each. Will not pack it.
    TEST_ASSERT(sizeof(EPS_struct_pdu_housekeeping_data_eng_t) == (258-6));
    TEST_ASSERT(sizeof(EPS_struct_pbu_housekeeping_data_eng_t) == (84-6));
    TEST_ASSERT(sizeof(EPS_struct_pcu_housekeeping_data_eng_t) == (72-6));
    TEST_ASSERT(sizeof(EPS_struct_piu_housekeeping_data_eng_t) == (274-6));
    
    return 0;
}
