#include "main.h"
#include "rtos_tasks/rtos_task_helpers.h"
#include "rtos_tasks/gps_rtos_tasks.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "gps/gps_types.h"
#include "gps/gps.h"
#include "gps/gps_ascii_parsers.h"

#include "cmsis_os.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

void TASK_receive_gps_info(void *argument)
{
    TASK_HELP_start_of_task();

    // CONFIGURATION PARAMETER
    uint32_t GPS_receive_timeout_duration_ms = 50;

    // TODO: Create an internal drivers folder and add a COMMAND Mode Function (Send a log command and receive a response, look into EPS_send_cmd_get_response)
    char latest_gps_response[UART_gps_buffer_len];
    uint16_t latest_gps_response_len = 0;

    while (1)
    {

        osDelay(200);

        LOG_message(
        		LOG_SYSTEM_GPS,
        		LOG_SEVERITY_NORMAL,
        		LOG_SINK_ALL,
        		"GPS Buffer Data: %s Write Index %d",
        		UART_gps_buffer,
        		UART_gps_buffer_write_idx
        	);

        // Checking if there is data in the GPS buffer
        if (UART_gps_buffer_write_idx > 0)
        {

            // Checking if new data has come ie No more data is being transmitted
            if ((HAL_GetTick() - UART_gps_last_write_time_ms > GPS_receive_timeout_duration_ms) && (UART_gps_buffer_write_idx > 0))
            {
                // Copy data from the UART buffer to a separate buffer
                latest_gps_response_len = UART_gps_buffer_write_idx;
                for (uint16_t i = 0; i < UART_gps_buffer_len; i++)
                {
                    latest_gps_response[i] = (char)UART_gps_buffer[i];
                }

                // Set the null terminator at the end of the `latest_gps_response` str.
                latest_gps_response[latest_gps_response_len] = '\0';

                // Clear the buffer (memset to 0, but volatile-compatible) and reset the write pointer.
                for (uint16_t i = 0; i < UART_gps_buffer_len; i++)
                {
                    UART_gps_buffer[i] = 0;
                }

                UART_gps_buffer_write_idx = 0;
            }

            LOG_message(
                LOG_SYSTEM_GPS,
                LOG_SEVERITY_NORMAL,
                LOG_SINK_ALL,
                "GPS Buffer Data (Should be empty): %s Write Index (Should be 0) %d",
                UART_gps_buffer,
                UART_gps_buffer_write_idx);

            // Parsing the gps data
            GPS_header_response_t gps_header;
            const uint8_t gps_header_result = GPS_header_response_parser(latest_gps_response, &gps_header);

            // TODO: Figure out what to do after this
            // Parse may have failed due to incomplete data
            // How do I handle the incomplete data? Could drop the data
            if (gps_header_result == 0)
            {
                continue;
            }

            GPS_bestxyza_response_t bestxyza_data;
            if (strcmp(gps_header.log_name, "BESTXYZA") == 0)
            {
                const uint8_t bestxyza_parse_result = GPS_bestxyza_data_parser(latest_gps_response, &bestxyza_data);

                if (bestxyza_parse_result == 0)
                {
                    continue;
                }
            }

            GPS_timea_response_t timea_data;
            if (strcmp(gps_header.log_name, "TIMEA") == 0)
            {
                const uint8_t timea_parse_result = GPS_timea_data_parser(latest_gps_response, &timea_data);

                if (timea_parse_result == 0)
                {
                    continue;
                }
            }
        }
    } /* End Task's Main Loop */
}
