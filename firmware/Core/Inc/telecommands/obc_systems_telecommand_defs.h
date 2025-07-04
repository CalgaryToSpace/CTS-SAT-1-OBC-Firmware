#ifndef INCLUDE_GUARD__TEMPERATURE_SENSOR_TELECOMMAND_DEFS_H__
#define INCLUDE_GUARD__TEMPERATURE_SENSOR_TELECOMMAND_DEFS_H__

#include "telecommand_exec/telecommand_definitions.h"


uint8_t TCMDEXEC_obc_read_temperature_complex(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_obc_read_temperature(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);

uint8_t TCMDEXEC_obc_adc_read_vbat_voltage(
    const char *args_str,
    char *response_output_buf, uint16_t response_output_buf_len
);


#endif // INCLUDE_GUARD__TEMPERATURE_SENSOR_TELECOMMAND_DEFS_H__
