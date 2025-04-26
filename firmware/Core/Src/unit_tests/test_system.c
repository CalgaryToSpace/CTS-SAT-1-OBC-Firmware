#include "unit_tests/test_system.h"
#include "system/system_low_power_mode.h"
#include "unit_tests/unit_test_helpers.h"
#include "log/log.h"
#include <string.h>


uint8_t TEST_EXEC__SYS_low_power_mode_error_enum_to_string(void)
{
    // GPS
    SYS_low_power_mode_error_enum_t gps_error = SYS_LOW_POWER_MODE_ERROR_GPS;
    const char* gps_expected_string = "GPS";
    const char* gps_str = SYS_low_power_mode_error_enum_to_string(gps_error);
    TEST_ASSERT(strcmp(gps_str, gps_expected_string) == 0);

    // ADCS
    SYS_low_power_mode_error_enum_t adcs_error = SYS_LOW_POWER_MODE_ERROR_ADCS;
    const char* adcs_expected_string = "ADCS";
    const char* adcs_str = SYS_low_power_mode_error_enum_to_string(adcs_error);
    TEST_ASSERT(strcmp(adcs_str, adcs_expected_string) == 0);

    // MPI 5V
    SYS_low_power_mode_error_enum_t mpi_5v_error = SYS_LOW_POWER_MODE_ERROR_MPI_5V;
    const char* mpi_5v_expected_string = "MPI_5V";
    const char* mpi_5v_str = SYS_low_power_mode_error_enum_to_string(mpi_5v_error);
    TEST_ASSERT(strcmp(mpi_5v_str, mpi_5v_expected_string) == 0);

    // MPI 12V
    SYS_low_power_mode_error_enum_t mpi_12v_error = SYS_LOW_POWER_MODE_ERROR_MPI_12V;
    const char* mpi_12v_expected_string = "MPI_12V";
    const char* mpi_12v_str = SYS_low_power_mode_error_enum_to_string(mpi_12v_error);
    TEST_ASSERT(strcmp(mpi_12v_str, mpi_12v_expected_string) == 0);

    // Camera
    SYS_low_power_mode_error_enum_t camera_error = SYS_LOW_POWER_MODE_ERROR_CAMERA;
    const char* camera_expected_string = "CAMERA";
    const char* camera_str = SYS_low_power_mode_error_enum_to_string(camera_error);
    TEST_ASSERT(strcmp(camera_str, camera_expected_string) == 0);

    // BOOM 12V
    SYS_low_power_mode_error_enum_t boom_error = SYS_LOW_POWER_MODE_ERROR_BOOM_12V;
    const char* boom_expected_string = "BOOM_12V";
    const char* boom_str = SYS_low_power_mode_error_enum_to_string(boom_error);
    TEST_ASSERT(strcmp(boom_str, boom_expected_string) == 0);

    // BOOM PINS
    SYS_low_power_mode_error_enum_t boom_pins_error = SYS_LOW_POWER_MODE_ERROR_BOOM_PINS;
    const char* boom_pins_expected_string = "BOOM_PINS";
    const char* boom_pins_str = SYS_low_power_mode_error_enum_to_string(boom_pins_error);
    TEST_ASSERT(strcmp(boom_pins_str, boom_pins_expected_string) == 0);

    // Unknown 
    SYS_low_power_mode_error_enum_t invalid_error = 0xFF;
    const char* invalid_expected_string = "Unknown";
    const char* invalid_str = SYS_low_power_mode_error_enum_to_string(invalid_error);
    TEST_ASSERT(strcmp(invalid_str, invalid_expected_string) == 0);
    
    return 0;
}
uint8_t TEST_EXEC__SYS_low_power_mode_result_to_json(void)
{
    char buffer[LOW_POWER_MODE_JSON_STRING_LEN] = {0};

    // Test case 1: All systems successfully disabled
    const char *all_success_json = 
    "{\"MPI_5V\":\"Successfully Disabled\", "
    "\"MPI_12V\":\"Successfully Disabled\", "
    "\"CAMERA\":\"Successfully Disabled\", "
    "\"BOOM_12V\":\"Successfully Disabled\", "
    "\"BOOM_PINS\":\"Successfully Disabled Boom Pins\", "
    "\"GPS\":\"Successfully Set Low Power Mode\", "
    "\"ADCS\":\"Successfully Set Low Power Mode\"}";

    SYS_low_power_mode_error_enum_t all_success = 0;
    uint8_t all_success_result = SYS_low_power_mode_result_to_json(all_success, buffer, sizeof(buffer));
    TEST_ASSERT(all_success_result == 0);
    TEST_ASSERT(strcmp(buffer, all_success_json) == 0);

    memset(buffer, 0, sizeof(buffer));

    // Test case 2: All systems failed
    const char *all_error_json = 
    "{\"MPI_5V\":\"Error Disabling\", "
    "\"MPI_12V\":\"Error Disabling\", "
    "\"CAMERA\":\"Error Disabling\", "
    "\"BOOM_12V\":\"Error Disabling\", "
    "\"BOOM_PINS\":\"Error Disabling Boom Pins\", "
    "\"GPS\":\"Error Setting Low Power Mode\", "
    "\"ADCS\":\"Error Setting Low Power Mode\"}";

    SYS_low_power_mode_error_enum_t all_error = SYS_LOW_POWER_MODE_ERROR_MPI_5V | 
                                               SYS_LOW_POWER_MODE_ERROR_MPI_12V | 
                                               SYS_LOW_POWER_MODE_ERROR_CAMERA | 
                                               SYS_LOW_POWER_MODE_ERROR_BOOM_12V | 
                                               SYS_LOW_POWER_MODE_ERROR_BOOM_PINS | 
                                               SYS_LOW_POWER_MODE_ERROR_GPS | 
                                               SYS_LOW_POWER_MODE_ERROR_ADCS;
    uint8_t all_error_result = SYS_low_power_mode_result_to_json(all_error, buffer, sizeof(buffer));
    TEST_ASSERT(all_error_result == 0);
    TEST_ASSERT(strcmp(buffer, all_error_json) == 0);

    memset(buffer, 0, sizeof(buffer));

    // Test case 3.0: Mixed results, ADCS and MPI 12V failed
    const char *adcs_mpi12v_error_json = 
    "{\"MPI_5V\":\"Successfully Disabled\", "
    "\"MPI_12V\":\"Error Disabling\", "
    "\"CAMERA\":\"Successfully Disabled\", "
    "\"BOOM_12V\":\"Successfully Disabled\", "
    "\"BOOM_PINS\":\"Successfully Disabled Boom Pins\", "
    "\"GPS\":\"Successfully Set Low Power Mode\", "
    "\"ADCS\":\"Error Setting Low Power Mode\"}";

    SYS_low_power_mode_error_enum_t adcs_mpi12v_error = SYS_LOW_POWER_MODE_ERROR_ADCS | SYS_LOW_POWER_MODE_ERROR_MPI_12V;
    uint8_t adcs_mpi12v_result = SYS_low_power_mode_result_to_json(adcs_mpi12v_error, buffer, sizeof(buffer));
    TEST_ASSERT(adcs_mpi12v_result == 0);
    TEST_ASSERT(strcmp(buffer, adcs_mpi12v_error_json) == 0);

    memset(buffer, 0, sizeof(buffer));

    // Test case 3.1: Mixed results, BOOM Pins, and GPS failed
    const char *boom_gps_error_json = 
    "{\"MPI_5V\":\"Successfully Disabled\", "
    "\"MPI_12V\":\"Successfully Disabled\", "
    "\"CAMERA\":\"Successfully Disabled\", "
    "\"BOOM_12V\":\"Successfully Disabled\", "
    "\"BOOM_PINS\":\"Error Disabling Boom Pins\", "
    "\"GPS\":\"Error Setting Low Power Mode\", "
    "\"ADCS\":\"Successfully Set Low Power Mode\"}";

    SYS_low_power_mode_error_enum_t boom_gps_error = SYS_LOW_POWER_MODE_ERROR_BOOM_PINS | SYS_LOW_POWER_MODE_ERROR_GPS;
    uint8_t boom_gps_result = SYS_low_power_mode_result_to_json(boom_gps_error, buffer, sizeof(buffer));
    TEST_ASSERT(boom_gps_result == 0);
    TEST_ASSERT(strcmp(buffer, boom_gps_error_json) == 0);

    memset(buffer, 0, sizeof(buffer));

    // Test case 4: Zero buffer size
    SYS_low_power_mode_error_enum_t zero_size_error = SYS_LOW_POWER_MODE_ERROR_MPI_5V;
    uint8_t zero_size_result = SYS_low_power_mode_result_to_json(zero_size_error, buffer, 0);
    TEST_ASSERT(zero_size_result == 1); // Expecting failure due to zero buffer size
    TEST_ASSERT(strcmp(buffer, "") == 0); // Buffer should remain empty

    // Test case 5: Buffer too small
    char small_buffer[10] = {0};
    SYS_low_power_mode_error_enum_t small_buffer_error = SYS_LOW_POWER_MODE_ERROR_GPS;
    uint8_t small_buffer_result = SYS_low_power_mode_result_to_json(small_buffer_error, small_buffer, sizeof(small_buffer));
    TEST_ASSERT(small_buffer_result == 1);
    TEST_ASSERT(strcmp(small_buffer, "") == 0); // Buffer should remain empty

    // Test case 6: NULL buffer
    SYS_low_power_mode_error_enum_t null_buffer_error = SYS_LOW_POWER_MODE_ERROR_ADCS;
    uint8_t null_buffer_result = SYS_low_power_mode_result_to_json(null_buffer_error, NULL, sizeof(buffer));
    TEST_ASSERT(null_buffer_result == 1); // Expecting failure due to NULL buffer

    return 0;
}