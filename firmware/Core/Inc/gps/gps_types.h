#ifndef INCLUDE_GUARD__GPS_TYPES_H
#define INCLUDE_GUARD__GPS_TYPES_H

#include <stdint.h>

// TODO: Confirm for the structs if the status_code is needed i.e. is it the binary version of the response 

// ****************** SECTION: gps_header_structs ******************

/// @brief  This refers to the status that indicates how well a time is known
/// All reported receiver times are subject to a qualifying time status
/// Refer to page 51-53 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    int gps_reference_time_status_decimal;
    const char *gps_reference_time_status_ascii;
} gps_reference_time_status;

/// @brief This is the first part of the response that every command response from the GNSS receiver has.
/// Refer to page 34-35 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char log_name[128];

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

    /// @brief Reserved for GPS internal use
    uint32_t reserved;

    /// @brief representing the receiver software build number
    uint32_t rx_sw_version;
} gps_response_header;

// ****************** END SECTION: gps_header_structs ******************


// ****************** SECTION: gps_data_structs ******************

/// @brief  This refers to the solution status
/// Refer to table 90 page 500-501 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    int binary;
    const char *gps_solution_status_ascii;
} gps_solution_status;

/// @brief  This refers to the position or velocity type
/// Refer to table 91 page 501-503 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    int gps_type_binary;
    const char *gps_type_ascii;
} gps_position_velocity_time_status;

/// @brief This is the struct for the BESTXYZA Command response.
/// Refer to page 515-517 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief Position solution status
    gps_solution_status position_solution_status;

    /// @brief Position type.
    gps_position_velocity_time_status position_type;

    /// @brief Documentation says the point coordinates come as a double. 
    /// We store as an int32 because the eatrths radius(6.10^6 m) fits within an int32.
    /// Position coordinates
    int32_t position_x_m;
    int32_t position_y_m;
    int32_t position_z_m;

    /// @brief Documentation says the standard deviation of the position coordinates come as a float. 
    /// The standard deviation of the position coordinates
    uint32_t position_x_std_m;
    uint32_t position_y_std_m;
    uint32_t position_z_std_m;

    /// @brief Velocity Solution status.
    gps_solution_status velocity_solution_status;

    /// @brief Velocity type. 
    gps_position_velocity_time_status velocity_type;

    /// @brief Documentation says the velocity vector come as a double. 
    /// The Velocity Vectors along the axes
    int64_t velocity_x_m_per_s;
    int64_t velocity_y_m_per_s;
    int64_t velocity_z_m_per_s;

    /// @brief Documentation says the standard deviation of the velocity vector come as a float. 
    /// The standard deviation of the velocity vector
    uint32_t velocity_x_std_m_per_s;
    uint32_t velocity_y_std_m_per_s;
    uint32_t velocity_z_std_m_per_s;

    /// @brief Base station identification. 
    char stn_id [4];

    /// @brief A measure of the latency in the velocity time tag in seconds. 
    /// It should be subtracted from the time to give improved results. 
    uint32_t velocity_latency;

    /// @brief Differential age in seconds. 
    uint32_t differential_age_sec;

    /// @brief Solution age in seconds. 
    uint32_t solution_age_sec;

    /// @brief Number of satellites tracked.
    uint8_t satellite_no_tracked;

    /// @brief Number of satellites used in solution.
    uint8_t satellite_no_used_in_solution;

    /// @brief Number of satellites with L1/E1/B1 signals used in solution. 
    uint8_t satellite_no_l1_e1_b1;

    /// @brief Number of satellites with multi-frequency signals used in solution 
    uint8_t satellite_no_sol_multifreq;
 
    char reserved;

    /// @brief Extended solution status. 
    /// Refer to table 94 on page 504 of the OEM& Commands and Logs Reference Manual
    uint8_t extended_solution_status;

    /// @brief Galileo and BeiDou signals used mask
    /// Refer to table 93 on page 503 of the OEM& Commands and Logs Reference Manual
    uint8_t galileo_beiDou_sig_mask;

    /// @brief GPS and GLONASS signals used mask.
    /// Refer to table 92 on page 503 of the OEM& Commands and Logs Reference Manual
    uint8_t gps_glonass_sig_mask;

    /// @brief 32 bit CRC (ASCII and Binary only). 
    uint32_t crc;
    
} bestxyza_response;

#endif // INCLUDE_GUARD__GPS_TYPES_H
