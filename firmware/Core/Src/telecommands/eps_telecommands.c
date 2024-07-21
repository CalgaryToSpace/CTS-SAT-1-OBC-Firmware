#include <eps_drivers/eps_commands.h>
#include <eps_drivers/eps_types.h>
#include <eps_drivers/eps_types_to_json.h>

#include <telecommands/eps_telecommands.h>

#include <stdio.h>

/// @brief Triggers the EPS watchdog. No args.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_watchdog(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    const uint8_t result = EPS_CMD_watchdog();

    if (result != 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS watchdog trigger failed (err %d)", result);
        return 1;
    }

    snprintf(response_output_buf, response_output_buf_len, "EPS watchdog triggered successfully.");
    return 0;
}

/// @brief Gets the EPS system status, and returns it as a JSON string.
/// @return 0 on success, >0 on failure.
uint8_t TCMDEXEC_eps_get_system_status_json(const char *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    EPS_result_system_status_t status;
    const uint8_t result = EPS_CMD_get_system_status(&status);

    if (result != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS get system status failed (err %d)", result);
        return 1;
    }

    const uint8_t result_json = EPS_result_system_status_TO_json(
        &status, response_output_buf, response_output_buf_len);

    if (result_json != 0) {
        snprintf(response_output_buf, response_output_buf_len,
            "EPS get system status JSON failed (err %d)", result_json);
        return 2;
    }
    return 0;
}
