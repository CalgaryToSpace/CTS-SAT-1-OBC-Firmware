#ifndef INCLUDE_GUARD__COMPLETE_SELF_CHECK_H
#define INCLUDE_GUARD__COMPLETE_SELF_CHECK_H

#include <stdint.h>

typedef struct {
    uint8_t obc_temperature_works;
    uint8_t is_adcs_i2c_addr_alive;
    uint8_t is_adcs_alive;
    uint8_t is_ax100_i2c_addr_alive;
    // uint8_t is_ax100_alive; // TODO: Is there a way to test this?
    uint8_t is_gnss_responsive;
    uint8_t is_eps_responsive;
    uint8_t is_eps_thriving;
    uint8_t mpi_science_rx;
    uint8_t mpi_cmd_works;
    uint8_t is_camera_responsive;

    uint8_t is_antenna_i2c_addr_a_alive;
    uint8_t is_antenna_i2c_addr_b_alive;
    uint8_t is_antenna_a_alive;
    uint8_t is_antenna_b_alive;

    uint8_t flash_0_alive;
    uint8_t flash_1_alive;
    uint8_t flash_2_alive;
    uint8_t flash_3_alive;

    uint8_t eps_no_overcurrent_faults;

    // Note: Skipping the automated check for the boom here.
} CTS1_system_self_check_result_struct_t;

void CTS1_run_system_self_check(CTS1_system_self_check_result_struct_t *result);

void CTS1_self_check_struct_TO_json_list(
    CTS1_system_self_check_result_struct_t self_check_struct,
    char dest_json_str[], uint16_t dest_json_str_size,
    uint8_t show_passes
);

#endif // INCLUDE_GUARD__COMPLETE_SELF_CHECK_H
