/*
 * mpi_types.h
 *
 * Created on: Jun 14, 2024
 * Author: Vaibhav Kapoor
 */

#ifndef __INCLUDE_GUARD__MPI_TYPES_H
#define __INCLUDE_GUARD__MPI_TYPES_H
#include <stdint.h>

/**
 * @enum Mpi_Mode
 * @brief Enumerates the different modes under which the satellite can operate the MPI
 * 
 */
enum Mpi_Mode {
    COMMAND_MODE,                           // MPI is in command mode
    SCIENCE_DATA_MODE,                      // MPI is science data is being collected mode
    NOT_LISTENING_TO_MPI_MODE               // MPI is recording science data but it is not being collected mode
};

extern enum Mpi_Mode current_mpi_mode;      // Current mode under which the MPI is being operated

/**
 * @brief Represents an data frame received from the MPI
 *
 * This struct lays out the structure of an MPI data frame,
 * which consists of synchronization bytes, frame counter, housekeeping telemetry,
 * image pixel data, and cyclic redundancy check (CRC) bytes.
 */
typedef struct
{
    uint8_t sync_byte1;                      // First synchronization byte
    uint8_t sync_byte2;                      // Second synchronization byte
    uint8_t sync_byte3;                      // Third synchronization byte
    uint8_t sync_byte4;                      // Fourth synchronization byte
    uint16_t frame_counter;                  // Frame counter for the data frame
    int16_t board_temperature;               // Current board temperature
    uint8_t firmware_version;                // Firmware version of the MPI unit
    uint8_t mpi_unit_id;                     // ID of the MPI unit
    uint16_t detector_status;                // Current status of the detector
    uint16_t inner_dome_voltage_setting;     // Voltage setting for the inner dome
    uint8_t spib_reserved;                   // Reserved field
    uint8_t inner_dome_scan_index;           // Index for inner dome scanning
    uint16_t faceplate_voltage_setting;      // Voltage setting for the faceplate
    uint16_t faceplate_voltage_adc_reading;  // ADC reading for faceplate voltage
    uint16_t inner_dome_voltage_adc_reading; // ADC reading for inner dome voltage
    uint16_t pixels[67];                     // Array of 67 image pixels    (Not a typo: It is actually 67!)
    uint16_t cyclic_redundancy_check;        // CRC for data integrity check
} MpiFrame_t;

#endif // __INCLUDE_GUARD__MPI_TYPES_H