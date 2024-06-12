#include <eps_drivers/eps_commands.h>
#include <eps_drivers/eps_types.h>

#include <telecommands/eps_telecommands.h>

/// @brief Triggers the EPS watchdog. No args.
/// @return 0 on success, 1 on failure.
uint8_t TCMDEXEC_eps_watchdog(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    
    const uint8_t result = EPS_CMD_watchdog();

    if (result == 0) {
        snprintf(response_output_buf, response_output_buf_len, "EPS watchdog triggered successfully.");
    } else {
        snprintf(
            response_output_buf, response_output_buf_len,
            "EPS watchdog trigger failed (err %d)", result);
    }
}
