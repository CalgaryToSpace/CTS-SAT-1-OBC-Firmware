#ifndef __INCLUDE_GUARD__EPS_TELECOMMANDS_H__
#define __INCLUDE_GUARD__EPS_TELECOMMANDS_H__

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"


uint8_t TCMDEXEC_eps_watchdog(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

// NOTE: Moved EPS_get_system_status
// Ebube's Addition ---------------------------------------------------------------------------------------------------

uint8_t TCMDEXEC_eps_vpid_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_vpid_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_battery_pack_datatype_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_battery_pack_datatype_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_conditioning_channel_datatype_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_conditioning_channel_datatype_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_conditioning_channel_short_datatype_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


uint8_t TCMDEXEC_eps_conditioning_channel_short_datatype_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


// NOTE: Moved function over here***

uint8_t TCMDEXEC_eps_get_system_status_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

// --------------------------------------

uint8_t TCMDEXEC_eps_result_pdu_overcurrent_fault_state_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

                        
uint8_t TCMDEXEC_eps_result_pbu_abf_placed_state_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


uint8_t TCMDEXEC_eps_result_pdu_housekeeping_data_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


uint8_t TCMDEXEC_eps_result_pdu_housekeeping_data_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);


uint8_t TCMDEXEC_eps_result_pbu_housekeeping_data_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_result_pbu_housekeeping_data_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_result_pcu_housekeeping_data_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_result_pcu_housekeeping_data_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_result_piu_housekeeping_data_raw_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_eps_result_piu_housekeeping_data_eng_json(const uint8_t *args_str,
                        TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif /* __INCLUDE_GUARD__EPS_TELECOMMANDS_H__ */

