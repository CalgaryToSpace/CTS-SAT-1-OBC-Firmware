#ifndef INCLUDE_GUARD__COMPLETE_SELF_CHECK_H
#define INCLUDE_GUARD__COMPLETE_SELF_CHECK_H

#include <stdint.h>

typedef struct {
    uint8_t obc_temperature_works;
    int32_t obc_temperature_deg_cC;
    uint8_t is_adcs_i2c_addr_alive;
    uint8_t is_adcs_alive;
    uint8_t is_ax100_i2c_addr_alive;
    // uint8_t is_ax100_alive; // TODO: Is there a way to test this?
    uint8_t is_gnss_responsive;
    uint8_t is_eps_responsive;
    uint8_t is_eps_thriving;
    uint8_t is_mpi_dumping;
    uint8_t mpi_cmd_works;
    uint8_t is_camera_responsive;

    uint8_t flash_0_alive;
    uint8_t flash_1_alive;
    uint8_t flash_2_alive;
    uint8_t flash_3_alive;

    // Note: Skipping the automated check for the boom here.
} CTS1_system_self_check_result_struct_t;

#endif // INCLUDE_GUARD__COMPLETE_SELF_CHECK_H
