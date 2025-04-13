#include "unit_tests/test_system.h"
#include "system/system_safe_mode.h"
#include "unit_tests/unit_test_helpers.h"
#include <string.h>
#include "test_system.h"


uint8_t TEST_EXEC__safe_mode_error_enum_to_string(void)
{
    // GPS
    SYS_safe_mode_error_enum_t gps_error = SYS_SAFE_MODE_ERROR_GPS;
    const char* gps_expected_string = "GPS";
    const char* gps_str = SYS_safe_mode_error_enum_to_string(gps_error);
    TEST_ASSERT(strcmp(gps_str, gps_expected_string) == 0);

    // ADCS
    SYS_safe_mode_error_enum_t adcs_error = SYS_SAFE_MODE_ERROR_ADCS;
    const char* adcs_expected_string = "ADCS";
    const char* adcs_str = SYS_safe_mode_error_enum_to_string(adcs_error);
    TEST_ASSERT(strcmp(adcs_str, adcs_expected_string) == 0);

    // MPI 5V
    SYS_safe_mode_error_enum_t mpi_5v_error = SYS_SAFE_MODE_ERROR_MPI_5V;
    const char* mpi_5v_expected_string = "MPI 5V";
    const char* mpi_5v_str = SYS_safe_mode_error_enum_to_string(mpi_5v_error);
    TEST_ASSERT(strcmp(mpi_5v_str, mpi_5v_expected_string) == 0);

    // MPI 12V
    SYS_safe_mode_error_enum_t mpi_12v_error = SYS_SAFE_MODE_ERROR_MPI_12V;
    const char* mpi_12v_expected_string = "MPI 12V";
    const char* mpi_12v_str = SYS_safe_mode_error_enum_to_string(mpi_12v_error);
    TEST_ASSERT(strcmp(mpi_12v_str, mpi_12v_expected_string) == 0);

    // Boom
    SYS_safe_mode_error_enum_t camera_error = SYS_SAFE_MODE_ERROR_CAMERA;
    const char* camera_expected_string = "Camera";
    const char* camera_str = SYS_safe_mode_error_enum_to_string(camera_error);
    TEST_ASSERT(strcmp(camera_str, camera_expected_string) == 0);

    // Camera
    SYS_safe_mode_error_enum_t boom_error = SYS_SAFE_MODE_ERROR_BOOM;
    const char* boom_expected_string = "Boom";
    const char* boom_str = SYS_safe_mode_error_enum_to_string(boom_error);
    TEST_ASSERT(strcmp(boom_str, boom_expected_string) == 0);

    // Unknown 
    SYS_safe_mode_error_enum_t invalid_error = 0xFF;
    const char* invalid_expected_string = "Unknown";
    const char* invalid_str = SYS_safe_mode_error_enum_to_string(invalid_error);
    TEST_ASSERT(strcmp(invalid_str, invalid_expected_string) == 0);
    
    return 0;
}
uint8_t TEST_EXEC__safe_mode_error_result_to_json(void)
{
    char buffer[256] = {0};

    // Test case 1: All systems successfully disabled
    const char *all_success_json = "{\"GPS\":\"Successfully Disabled\", \"ADCS\":\"Successfully Disabled\", \"MPI 5V\":\"Successfully Disabled\", \"MPI 12V\":\"Successfully Disabled\", \"Camera\":\"Successfully Disabled\", \"Boom\":\"Successfully Disabled\"}";
    SYS_safe_mode_error_enum_t all_success = 0;
    uint8_t all_success_result = SYS_safe_mode_error_result_to_json(all_success, buffer, sizeof(buffer));
    TEST_ASSERT(all_success_result == 0);
    TEST_ASSERT(strcmp(buffer, all_success_json) == 0);

    memset(buffer, 0, sizeof(buffer));

    // Test case 2: All systems failed to disable
    const char *all_error_json = "{\"GPS\":\"Error Disabling\", \"ADCS\":\"Error Disabling\", \"MPI 5V\":\"Error Disabling\", \"MPI 12V\":\"Error Disabling\", \"Camera\":\"Error Disabling\", \"Boom\":\"Error Disabling\"}";
    SYS_safe_mode_error_enum_t all_error = SYS_SAFE_MODE_ERROR_GPS | SYS_SAFE_MODE_ERROR_ADCS | SYS_SAFE_MODE_ERROR_MPI_5V | SYS_SAFE_MODE_ERROR_MPI_12V | SYS_SAFE_MODE_ERROR_CAMERA | SYS_SAFE_MODE_ERROR_BOOM;
    uint8_t all_error_result = SYS_safe_mode_error_result_to_json(all_error, buffer, sizeof(buffer));
    TEST_ASSERT(all_error_result == 0);
    TEST_ASSERT(strcmp(buffer, all_error_json) == 0);
   
    memset(buffer, 0, sizeof(buffer));
    
    // Test case 3: Mixed results, ADCS and MPI 12V failed
    const char *adcs_mpi12v_error_json = "{\"GPS\":\"Successfully Disabled\", \"ADCS\":\"Error Disabling\", \"MPI 5V\":\"Successfully Disabled\", \"MPI 12V\":\"Error Disabling\", \"Camera\":\"Successfully Disabled\", \"Boom\":\"Successfully Disabled\"}";
    SYS_safe_mode_error_enum_t adcs_mpi12v_error = SYS_SAFE_MODE_ERROR_ADCS | SYS_SAFE_MODE_ERROR_MPI_12V;
    uint8_t adcs_mpi12v_result = SYS_safe_mode_error_result_to_json(adcs_mpi12v_error, buffer, sizeof(buffer));
    TEST_ASSERT(adcs_mpi12v_result == 0);
    TEST_ASSERT(strcmp(buffer, adcs_mpi12v_error_json) == 0);

    memset(buffer, 0, sizeof(buffer));

    // Test case 4: Zero buffer size
    SYS_safe_mode_error_enum_t zero_size_error = SYS_SAFE_MODE_ERROR_MPI_5V;
    uint8_t zero_size_result = SYS_safe_mode_error_result_to_json(zero_size_error, buffer, 0);
    TEST_ASSERT(zero_size_result == 1); // Expecting failure due to zero buffer size
    TEST_ASSERT(strcmp(buffer, "") == 0); // Buffer should remain empty


    // Test case 5: Buffer too small
    char small_buffer[10] = {0};
    SYS_safe_mode_error_enum_t small_buffer_error = SYS_SAFE_MODE_ERROR_GPS;
    uint8_t small_buffer_result = SYS_safe_mode_error_result_to_json(small_buffer_error, small_buffer, sizeof(small_buffer));
    TEST_ASSERT(small_buffer_result == 1);
    TEST_ASSERT(strcmp(small_buffer, "") == 0); // Buffer should remain empty

    // Test case 6: NULL buffer
    SYS_safe_mode_error_enum_t null_buffer_error = SYS_SAFE_MODE_ERROR_ADCS;
    uint8_t null_buffer_result = SYS_safe_mode_error_result_to_json(null_buffer_error, NULL, sizeof(buffer));
    TEST_ASSERT(null_buffer_result == 1); // Expecting failure due to NULL buffer

    return 0;
}