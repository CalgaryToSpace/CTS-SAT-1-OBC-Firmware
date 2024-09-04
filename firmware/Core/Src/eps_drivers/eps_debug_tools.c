
#include <string.h>
#include <stdio.h>

#include "debug_tools/debug_uart.h"
#include "eps_drivers/eps_debug_tools.h"
#include "eps_drivers/eps_commands.h"


void EPS_debug_uart_print_system_status(EPS_struct_system_status_t* system_status) {
	char msg1[365];
	sprintf(
	    msg1,
	    "Mode: %d, Configuration Changed?: %d, Reset cause: %d, Uptime: %lu sec, Error Code: %d, rst_cnt_pwron: %u, rst_cnt_wdg: %d, rst_cnt_cmd: %d, rst_cnt_mcu: %d, rst_cnt_emlopo: %d, time_since_prev_cmd: %d sec, Unix time: %lu sec, Unix year: %d, Unix month: %d, Unix day: %d, Unix hour: %d, Unix minute: %d, Unix second: %d\n",
	    system_status->mode, system_status->config_changed_since_boot, system_status->reset_cause,
        system_status->uptime_sec, system_status->error_code,
        system_status->rst_cnt_pwron, system_status->rst_cnt_wdg, system_status->rst_cnt_cmd,
        system_status->rst_cnt_mcu, system_status->rst_cnt_emlopo,
        
        system_status->time_since_prev_cmd_sec, system_status->unix_time_sec,
        (system_status->calendar_years_since_2000 + 2000), system_status->calendar_month, system_status->calendar_day,
        system_status->calendar_hour, system_status->calendar_minute, system_status->calendar_second
	);

	DEBUG_uart_print_str(msg1);
}
