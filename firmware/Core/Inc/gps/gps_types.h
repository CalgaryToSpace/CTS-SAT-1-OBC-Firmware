#ifndef INCLUDE_GUARD__GPS_TYPES_H
#define INCLUDE_GUARD__GPS_TYPES_H

#include <stdint.h>

/// @brief This is the first part of the response that every command response from the GNSS receiver has.
typedef struct {
    /// @brief Sync character. The ASCII message is always preceded by a single ‘#’ symbol
    char lead_code;

    /// @brief The ASCII name of the log or command e.g., "BESTXYZA" or "TIMEA"
    char log_name[255];

    /// @brief The name of the port from which the log was generated e.g., "COM1".
    char port[10];

    /// @brief Used for multiple related logs, e.g., 0.
    uint32_t sequence_no;

    /// @brief The minimum percentage of time the processor is idle.
    uint32_t idle_time;
    
} GPS_response;

#endif // INCLUDE_GUARD__GPS_TYPES_H
