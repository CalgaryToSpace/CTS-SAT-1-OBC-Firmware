#ifndef INCLUDE_GUARD__GPS_TYPES_H
#define INCLUDE_GUARD__GPS_TYPES_H

#include <stdint.h>

/// @brief  This refers to the status that indicates how well a time is known
/// All reported receiver times are subject to a qualifying time status
///Refer to page 51-53 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    int decimal;
    const char *ascii;
    const char *description;
} gps_reference_time_status;

/// @brief This is the first part of the response that every command response from the GNSS receiver has.
/// Refer to page 34-35 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char log_name[255];

    /// @brief The name of the port from which the log was generated e.g., "COM1".
    char port[10];

    /// @brief Used for multiple related logs, e.g., 0.
    uint32_t sequence_no;

    /// @brief The minimum percentage of time the processor is idle.
    uint32_t idle_time;

    /// @brief Indicates the quality of the GPS reference time
    gps_reference_time_status time_status;

    /// @brief GPS reference week number
    uint32_t week;

    /// @brief Seconds from the beginning of the GPS reference week
    uint32_t seconds;

    /// @brief An eight digit hexadecimal number representing the status of
    /// various hardware and software components of the receiver
    int32_t rx_status;

    /// @brief Reserved for internal use
    uint32_t reserved;

    /// @brief representing the receiver software build number
    uint32_t rx_sw_version;
} gps_response_header;


/// @brief This is the first part of the response that every command response from the GNSS receiver has.
typedef struct {
    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char p_sol_status;

    /// @brief The name of the port from which the log was generated e.g., "COM1".
    char pos_type;

    /// @brief Documentation says the point coordinates come as a double. 
    /// We store as an int32 because the eatrths radius(6.10^6 m) fits within an int32.
    int32_t p_x_m;
    int32_t p_y_m;
    int32_t p_z_m;

    uint32_t p_x_std;
    uint32_t p_y_std;
    uint32_t p_z_std;

    uint32_t v_sol_status;
    char vel_type;

    uint32_t v_x;
    uint32_t v_y;
    uint32_t v_z;

    uint32_t v_x_std;
    uint32_t v_y_std;
    uint32_t v_z_std;

    uint32_t stn_id;

    uint32_t v_latency;
    uint32_t diff_age;
    uint32_t sol_age;
    uint32_t satellite_no_tracked;
    uint32_t satellite_no_sol;
    uint32_t satellite_no_l1_e1_b1;
    uint32_t satellite_no_sol_multifreq;
    char reserved;
    uint32_t ex_sol_status;
    uint32_t galileo_beiDou_sig_mask;
    uint32_t gps_glonass_sig_mask;
    uint32_t crc;
    uint32_t terminator;

    /// @brief The minimum percentage of time the processor is idle.
    uint32_t idle_time;
    
} bestxyza_response;

#endif // INCLUDE_GUARD__GPS_TYPES_H
