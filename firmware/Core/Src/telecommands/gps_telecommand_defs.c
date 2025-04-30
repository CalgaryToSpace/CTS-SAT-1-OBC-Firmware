#include "telecommand_exec/telecommand_definitions.h"
#include "uart_handler/uart_handler.h"
#include "telecommands/eps_telecommands.h"
#include "gps/gps_internal_drivers.h"
#include "littlefs/littlefs_helper.h"
#include "timekeeping/timekeeping.h"
#include "log/log.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/// @brief Telecommand: Transmit a log command to the GPS receiver through UART
/// @param args_str
/// - Arg 0: Log command to be sent to GPS eg "log bestxyza once" (string)
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success, > 0 error
uint8_t TCMDEXEC_gps_send_cmd_ascii(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                  char *response_output_buf, uint16_t response_output_buf_len)
{

    if (args_str == NULL)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Empty args_str");
        return 1;
    }

    // TODO : Determine if we need to perform extra error checks on arg_str ie log command format etc

    // Adding a new line character to the log command
    char gps_log_cmd[128];
    snprintf(gps_log_cmd, sizeof(gps_log_cmd), "%s\n", args_str);
    const uint16_t gps_log_cmd_len = strlen(gps_log_cmd);

    // Allocate space to receive incoming GPS response.
    const uint16_t GPS_rx_buffer_max_size = 512;
    uint16_t GPS_rx_buffer_len = 0;
    uint8_t GPS_rx_buffer[GPS_rx_buffer_max_size];
    memset(GPS_rx_buffer, 0, GPS_rx_buffer_max_size); // Initialize all elements to 0

    // Send log command to GPS and receive response
    const uint8_t gps_cmd_response = GPS_send_cmd_get_response(
        gps_log_cmd, gps_log_cmd_len, GPS_rx_buffer, GPS_rx_buffer_len, GPS_rx_buffer_max_size
    );

    // Handle the gps_cmd_response: Perform the error checks
    // TODO: Potentially add GPS_validate_log_response function in here to validate response from the gps receiver

    if(gps_cmd_response != 0){
        LOG_message(
            LOG_SYSTEM_GPS,
            LOG_SEVERITY_NORMAL,
            LOG_SINK_ALL,
            "GPS Response Code: %d",
            gps_cmd_response
        );
    }

    LOG_message(
        LOG_SYSTEM_GPS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "GPS_rx_buffer: %s",
        GPS_rx_buffer
    );

    // Mount the Memory Module
    const int8_t mount_result = LFS_mount();
    if(mount_result !=0){
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LFS ERROR: Mounting error: %d",
            mount_result
        );

        // TODO: Handle Mounting Error. Treating it as a warning
    }

    // Create the gps file directory and if the directory is present, throw warning
    const int8_t dir_creation_response =  LFS_make_directory("gps_logs");
    if(dir_creation_response == -2){
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LFS WARNING: Dicrectory already exists"
        );
    }

    // Creating the file name to store the gps response using the timestamp of the file creation as an identifier
    char timeresponse [50];
    TIM_get_timestamp_string_datetime(timeresponse, sizeof(timeresponse));

    char gps_file_path[100];
    snprintf(gps_file_path, sizeof(gps_file_path), "gps_logs/%s.txt", timeresponse);

    // Wwrite GPS response to the designated file path
    const int8_t write_response = LFS_write_file(gps_file_path,GPS_rx_buffer,GPS_rx_buffer_len);
    if(write_response < 0 )
    {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LFS ERROR: Writing error: %d",
            write_response
        );
    }

    // Test through reading to verify write contents
    uint8_t test_read_buffer[512] = {0};
    const int8_t read_result = LFS_read_file(gps_file_path, 0, test_read_buffer, sizeof(test_read_buffer));
    if(read_result < 0 )
    {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LFS ERROR: Reading error: %d",
            read_result
        );
    }

    test_read_buffer[sizeof(test_read_buffer) - 1] = '\0'; 

    LOG_message(
        LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
        "LFS Successfully Read File '%s'. System uptime: %lu, File Content: '%s'!",
        gps_file_path, HAL_GetTick(), (char*)test_read_buffer
    );

    // Unmounting Memory Module
    const int8_t unmount_response = LFS_unmount();
    if(unmount_response < 0)
    {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "LFS ERROR: Unmounting error: %d",
            unmount_response
        );
    }

    snprintf(response_output_buf, response_output_buf_len, "GPS Command: '%s' successfully transmitted", args_str);

    return 0;
}