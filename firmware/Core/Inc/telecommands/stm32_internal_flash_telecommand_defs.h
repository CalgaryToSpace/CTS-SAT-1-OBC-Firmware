#ifndef INCLUDE_GUARD_STM32_INTERNAL_FLASH_TELECOMMAND_DEFS_H
#define INCLUDE_GUARD_STM32_INTERNAL_FLASH_TELECOMMAND_DEFS_H

#include "telecommand_exec/telecommand_types.h"

uint8_t TCMDEXEC_stm32_internal_flash_write(const char *args_str,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_stm32_internal_flash_read(const char *args_str,
                                           char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_stm32_internal_flash_erase(const char *args_str,
                                            char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_stm32_internal_flash_get_option_bytes(const char *args_str,
                                                       char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_stm32_internal_flash_set_active_flash_bank(const char *args_str,
                                                       char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_stm32_internal_flash_get_active_flash_bank(const char *args_str,
                                                       char *response_output_buf, uint16_t response_output_buf_len);                                                       
#endif /* INCLUDE_GUARD_STM32_INTERNAL_FLASH_TELECOMMAND_DEFS_H */
