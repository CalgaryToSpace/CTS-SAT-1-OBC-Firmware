#ifndef __INCLUDE__GUARD__LOG_SINKS_H_
#define __INCLUDE__GUARD__LOG_SINKS_H_

#define LOG_UART_TRANSMIT_TIMEOUT 200

void LOG_to_file(const char msg[]);
void LOG_to_umbilical_uart(const char msg[]);
void LOG_to_uhf_radio(const char msg[]);

#endif // __INCLUDE__GUARD__LOG_SINKS_H_
