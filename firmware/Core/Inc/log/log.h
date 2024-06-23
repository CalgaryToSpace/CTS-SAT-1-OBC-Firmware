#ifndef __INCLUDE__GUARD__LOG_H_
#define __INCLUDE__GUARD__LOG_H_

#include "log/ulog.h"

// Number of ticks to wait for the UART transmit to finish
#define LOG_UART_TRANSMIT_TIMEOUT 100 
#define LOG_MAX_LENGTH 255

#define LOG_SYSTEM_FILENAME "/system.log"

void LOG_init(void);

void LOG_prep_full_log_message(char *full_log_msg, ulog_level_t severity, char *msg);
void LOG_send_to_system_log_file(ulog_level_t severity, char *msg);
void LOG_send_to_uart(ulog_level_t severity, char *msg);
void LOG_send_to_radio(ulog_level_t severity, char *msg);


#endif // __INCLUDE__GUARD__LOG_H_

