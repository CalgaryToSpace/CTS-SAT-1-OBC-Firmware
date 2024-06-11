
#ifndef __INCLUDE_GUARD__EPS_DEBUG_TOOLS_H__
#define __INCLUDE_GUARD__EPS_DEBUG_TOOLS_H__

#include "eps_drivers/eps_types.h"

void EPS_debug_uart_print_system_status(EPS_result_system_status_t* system_status);

void EPS_result_pdu_housekeeping_data_eng_to_json(const EPS_result_pdu_housekeeping_data_eng_t *data, char json_output_str[]);

#endif /* __INCLUDE_GUARD__EPS_DEBUG_TOOLS_H__ */
