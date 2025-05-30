#ifndef INCLUDE_GUARD__TEST_LOGGING_H__
#define INCLUDE_GUARD__TEST_LOGGING_H__

#include <stdint.h>

uint8_t TEST_EXEC__LOG_enable_disable_LOG_SINK_FILE(void);
uint8_t TEST_EXEC__LOG_enable_disable_LOG_SINK_UHF_RADIO(void);
uint8_t TEST_EXEC__LOG_enable_disable_LOG_SINK_UMBILICAL_UART(void);
uint8_t TEST_EXEC__LOG_enable_disable_LOG_FILE_LOGGING_FOR_SYSTEM_MPI(void);
uint8_t TEST_EXEC__LOG_all_sinks_except(void);

#endif // INCLUDE_GUARD__TEST_LOGGING_H__
