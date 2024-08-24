#ifndef INCLUDE_GUARD__GPS_TYPES_H
#define INCLUDE_GUARD__GPS_TYPES_H

#include <stdint.h>

// ****************** SECTION: gps_header_structs ******************

/// @brief  This refers to the status that indicates how well a time is known
/// All reported receiver times are subject to a qualifying time status
/// Refer to page 51-53 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_UNKNOWN = 20,
    GPS_APPROXIMATE = 60,
    GPS_COARSEADJUSTING = 80,
    GPS_COARSE = 100,
    GPS_COARSESTEERING = 120,
    GPS_FREEWHEELING = 130,
    GPS_FINEADJUSTING = 140,
    GPS_FINE = 160,
    GPS_FINEBACKUPSTEERING = 170,
    GPS_FINESTEERING = 180,
    GPS_SATTIME = 200
} GPS_reference_time_status_t;


/// @brief This is the first part of the response that every command response from the GNSS receiver has.
/// Refer to page 34-35 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char log_name[128];

    /// @brief The name of the port from which the log was generated e.g., "COM1".
    char port[10];

    /// @brief Used for multiple related logs, e.g., 0.
    /// TODO: Potentially try use a fixed point number to get decimal points
    uint32_t sequence_no;

    /// @brief The minimum percentage of time the processor is idle.
    uint32_t idle_time;

    /// @brief Indicates the quality of the GPS reference time
    GPS_reference_time_status_t time_status;

    /// @brief GPS reference week number
    uint32_t week;

    /// @brief Seconds from the beginning of the GPS reference week
    /// TODO: Potentially try use a fixed point number to get decimal points
    uint32_t seconds;

    /// @brief An eight digit hexadecimal number representing the status of
    /// various hardware and software components of the receiver
    int32_t rx_status;

    /// @brief representing the receiver software build number
    uint32_t rx_sw_version;
} gps_response_header;

// ****************** END SECTION: gps_header_structs ******************


// ****************** SECTION: gps_data_structs ******************

/// @brief  This refers to the solution status
/// Refer to table 90 page 500-501 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_SOL_COMPUTED = 0,
    GPS_INSUFFICIENT_OBS = 1,
    GPS_NO_CONVERGENCE = 2,
    GPS_SINGULARITY = 3,
    GPS_COV_TRACE = 4,
    GPS_TEST_DIST = 5,
    GPS_COLD_START = 6,
    GPS_V_H_LIMIT = 7,
    GPS_VARIANCE = 8,
    GPS_RESIDUALS = 9,
    GPS_RESERVED_10_12 = 10,  // Since 10-12 are reserved
    GPS_INTEGRITY_WARNING = 13
    // 14-17 are reserved
} GPS_solution_status_enum_t;


/// @brief  This refers to the position or velocity type
/// Refer to table 91 page 501-503 of the OEM7 Commands and Logs Reference Manual
typedef enum {
    GPS_TYPE_NONE = 0,
    GPS_TYPE_FIXEDPOS = 1,
    GPS_TYPE_FIXEDHEIGHT = 2,
    GPS_TYPE_RESERVED_3_7 = 3,  // Values from 3 to 7 are reserved
    GPS_TYPE_DOPPLER_VELOCITY = 8,
    GPS_TYPE_RESERVED_9_15 = 9, // Values from 9 to 15 are reserved
    GPS_TYPE_SINGLE = 16,
    GPS_TYPE_PSDIFF = 17,
    GPS_TYPE_WAAS = 18,
    GPS_TYPE_PROPAGATED = 19,
    GPS_TYPE_RESERVED_20_31 = 20, // Values from 20 to 31 are reserved
    GPS_TYPE_L1_FLOAT = 32,
    GPS_TYPE_RESERVED_33 = 33,
    GPS_TYPE_NARROW_FLOAT = 34,
    GPS_TYPE_RESERVED_35_47 = 35, // Values from 35 to 47 are reserved
    GPS_TYPE_L1_INT = 48,
    GPS_TYPE_WIDE_INT = 49,
    GPS_TYPE_NARROW_INT = 50,
    GPS_TYPE_RTK_DIRECT_INS = 51,
    GPS_TYPE_INS_SBAS = 52,
    GPS_TYPE_INS_PSRSP = 53,
    GPS_TYPE_INS_PSRDIFF = 54,
    GPS_TYPE_INS_RTKFLOAT = 55,
    GPS_TYPE_INS_RTKFIXED = 56,
    GPS_TYPE_RESERVED_57_67 = 57, // Values from 57 to 67 are reserved
    GPS_TYPE_PPP_CONVERGING = 68,
    GPS_TYPE_PPP = 69,
    GPS_TYPE_OPERATIONAL = 70,
    GPS_TYPE_WARNING = 71,
    GPS_TYPE_OUT_OF_BOUNDS = 72,
    GPS_TYPE_INS_PPP_CONVERGING = 73,
    GPS_TYPE_INS_PPP = 74,
    GPS_TYPE_PPP_BASIC_CONVERGING = 77,
    GPS_TYPE_PPP_BASIC = 78,
    GPS_TYPE_INS_PPP_BASIC_CONVERGING = 79,
    GPS_TYPE_INS_PPP_BASIC = 80
} GPS_position_velocity_type_enum_t;


/// @brief This is the struct for the BESTXYZA Command response.
/// Refer to page 515-517 of the OEM7 Commands and Logs Reference Manual
typedef struct {
    /// @brief Position solution status
    GPS_solution_status_enum_t position_solution_status;

    /// @brief Position type.
    GPS_position_velocity_type_enum_t position_type;

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
    GPS_solution_status_enum_t velocity_solution_status;

    /// @brief Velocity type. 
    GPS_position_velocity_type_enum_t velocity_type;

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
