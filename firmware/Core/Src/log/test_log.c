#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdarg.h>

// This just includes a main function to show why the original indexing was not working.


typedef enum {
    LOG_SYSTEM_OBC = 1 << 0,
    LOG_SYSTEM_UHF_RADIO = 1 << 1,
    LOG_SYSTEM_UMBILICAL_UART = 1 << 2,
    LOG_SYSTEM_GPS = 1 << 3,
    LOG_SYSTEM_MPI = 1 << 4,
    LOG_SYSTEM_EPS = 1 << 5,
    LOG_SYSTEM_BOOM = 1 << 6,
    LOG_SYSTEM_ADCS = 1 << 7,
    LOG_SYSTEM_LFS = 1 << 8,
    LOG_SYSTEM_FLASH = 1 << 9,
    LOG_SYSTEM_ANTENNA_DEPLOY = 1 << 10,
    LOG_SYSTEM_LOG = 1 << 11,
    LOG_SYSTEM_TELECOMMAND = 1 << 12,
    LOG_SYSTEM_UNIT_TEST = 1 << 13,
    LOG_SYSTEM_UNKNOWN = 1 << 14,
    LOG_SYSTEM_ALL = (1 << 15) - 1,
} LOG_system_enum_t;

enum {
    LOG_SYSTEM_OFF = 0,
    LOG_SYSTEM_ON = 1,
};

// Internal interfaces and variables
#define LOG_TIMESTAMP_MAX_LENGTH 30
#define LOG_SINK_NAME_MAX_LENGTH 20
#define LOG_SYSTEM_NAME_MAX_LENGTH 20

typedef struct {
    LOG_system_enum_t system;
    char name[LOG_SYSTEM_NAME_MAX_LENGTH];
    char *log_file_path;
    uint8_t file_logging_enabled;
    uint32_t severity_mask;
} LOG_system_t;

// Severity masking.
static const uint8_t LOG_SEVERITY_MASK_ALL = 0xFF;
// TODO: Set mask to: No debugging messages by default.
// static const uint8_t LOG_SEVERITY_MASK_DEFAULT = LOG_SEVERITY_MASK_ALL & ~(uint8_t)LOG_SEVERITY_DEBUG;
static const uint8_t LOG_SEVERITY_MASK_DEFAULT = LOG_SEVERITY_MASK_ALL;

// Note: LOG_systems entries must have same order as LOG_system_enum_t
// entries.
static LOG_system_t LOG_systems[] = {
    {LOG_SYSTEM_OBC, "OBC", "/logs/obc_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UHF_RADIO, "UHF_RADIO", "/logs/uhf_radio_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UMBILICAL_UART, "UMBILICAL_UART", "/logs/umbilical_uart_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_GPS, "GPS", "/logs/gps_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_MPI, "MPI", "/logs/mpi_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_EPS, "EPS", "/logs/eps_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_BOOM, "BOOM", "/logs/boom_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_ADCS, "ADCS", "/logs/adcs_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_LFS, "LFS", "/logs/lfs_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_FLASH, "FLASH", "/logs/flash_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_ANTENNA_DEPLOY, "ANTENNA_DEPLOY", "/logs/antenna_deploy_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_LOG, "LOG", "/logs/log_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_TELECOMMAND, "TELECOMMAND", "/logs/telecommand_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
    {LOG_SYSTEM_UNIT_TEST, "UNIT_TEST", "/logs/unit_test_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
// LOG_SYSTEM_UNKNOWN must be the LAST entry to make it easy to find below
    {LOG_SYSTEM_UNKNOWN, "UNKNOWN", "/logs/unknown_system.log", LOG_SYSTEM_ON, LOG_SEVERITY_MASK_DEFAULT},
};
static const uint16_t LOG_NUMBER_OF_SYSTEMS = sizeof(LOG_systems) / sizeof(LOG_system_t);

int main() {
    LOG_system_enum_t gpsMask = LOG_SYSTEM_GPS;
    uint16_t GPSindex = __builtin_ctz(gpsMask);
    LOG_system_t *gps_system = &LOG_systems[GPSindex];
    printf("Direct Indexing GPS:\n");
    printf("Bitmask converted to index: %u\n", GPSindex);
    printf("System Name: %s\n", gps_system->name);
    printf("Log File Path: %s\n", gps_system->log_file_path);

    printf("****************\n");


    LOG_system_enum_t telecommandMask = LOG_SYSTEM_TELECOMMAND;
    uint16_t Teleindex = __builtin_ctz(telecommandMask);
    LOG_system_t *tele_system = &LOG_systems[Teleindex];
    printf("Direct Indexing Telecommand:\n");
    printf("Bitmask converted to index: %u\n", Teleindex);
    printf("System Name: %s\n", tele_system->name);
    printf("Log File Path: %s\n", tele_system->log_file_path);

    printf("****************\n");


    // This is what log system was doing originally
    // I believe this is undefined behavior since we are shifting 1 by 1 << 128 in this case (which exceeds the width of uint32) 
    LOG_system_enum_t ADCSMask = LOG_SYSTEM_ADCS;
    LOG_system_t *adcs_system = &LOG_systems[1 << ADCSMask];
    printf("Indexing ADCS with Bitshift:\n");
    printf("Bitmask converted to index: %u\n", (1 << ADCSMask));
    printf("System Name: %s\n", adcs_system->name);
    printf("Log File Path: %s\n", adcs_system->log_file_path);

    printf("****************\n");

    // Here we see using 1 << UHFMask is actually indexing into the MPI, since 1 << 1 << 1 is 4 
    LOG_system_enum_t UHFMask = LOG_SYSTEM_UHF_RADIO;
    LOG_system_t *uhf_system = &LOG_systems[1 << UHFMask];
    printf("Indexing UHF with Bitshift:\n");
    printf("Bitmask converted to index: %u\n", (1 << UHFMask));
    printf("System Name: %s\n", uhf_system->name);
    printf("Log File Path: %s\n", uhf_system->log_file_path);
    

};