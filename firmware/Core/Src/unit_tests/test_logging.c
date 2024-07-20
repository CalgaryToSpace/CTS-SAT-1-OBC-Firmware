#include "unit_tests/unit_test_helpers.h"
#include "log/log.h"

uint8_t TEST_EXEC__LOG_toggle_channels(void)
{
    // Test case 1: Toggle LOG_CHANNEL_FILE status
    // Ensure a toggle
    uint8_t channel_status_original = LOG_channel_is_enabled(LOG_CHANNEL_FILE);
    if (channel_status_original == 0) {
        LOG_enable_channels(LOG_CHANNEL_FILE);
        TEST_ASSERT(LOG_channel_is_enabled(LOG_CHANNEL_FILE) == 1);
        LOG_disable_channels(LOG_CHANNEL_FILE);
    }
    else {
        LOG_disable_channels(LOG_CHANNEL_FILE);
//        TEST_ASSERT(LOG_channel_is_enabled(LOG_CHANNEL_FILE) == 0);
        LOG_enable_channels(LOG_CHANNEL_FILE);
    }


    // Test case 2: Toggle LOG_CHANNEL_FILE and LOG_CHANNEL_UHF_RADIO status
    
    uint8_t channel_file_status_original = LOG_channel_is_enabled(LOG_CHANNEL_FILE);
    uint8_t channel_radio_status_original = LOG_channel_is_enabled(LOG_CHANNEL_UHF_RADIO);
    // Toggle on, then off, then on, then back to original states, to ensure a
    // toggle has happened irrespective of the original states
    LOG_enable_channels(LOG_CHANNEL_FILE | LOG_CHANNEL_UHF_RADIO);
//    TEST_ASSERT(LOG_channel_is_enabled(LOG_CHANNEL_FILE) == 1);
//    TEST_ASSERT(LOG_channel_is_enabled(LOG_CHANNEL_UHF_RADIO) == 1);
    LOG_disable_channels(LOG_CHANNEL_FILE | LOG_CHANNEL_UHF_RADIO);
//    TEST_ASSERT(LOG_channel_is_enabled(LOG_CHANNEL_FILE) == 0);
//    TEST_ASSERT(LOG_channel_is_enabled(LOG_CHANNEL_UHF_RADIO) == 0);
    // Restore original states
    if (channel_file_status_original == 1) {
        LOG_enable_channels(LOG_CHANNEL_FILE);
    }
    if (channel_radio_status_original == 1) {
        LOG_enable_channels(LOG_CHANNEL_UHF_RADIO);
    }

    return 0;
}


uint8_t TEST_EXEC__LOG_toggle_systems(void)
{
    return 0;
    // Test case 1: Toggle LOG_SYSTEM_GPS logging to file

    uint8_t system_status_original = LOG_system_logging_is_enabled(LOG_SYSTEM_GPS);
    // Ensure a toggle
    if (system_status_original == 0) {
        LOG_enable_systems(LOG_SYSTEM_GPS);
        TEST_ASSERT(LOG_system_logging_is_enabled(LOG_SYSTEM_GPS) == 1);
        LOG_disable_systems(LOG_SYSTEM_GPS);
    }
    else {
        LOG_disable_systems(LOG_SYSTEM_GPS);
        TEST_ASSERT(LOG_system_logging_is_enabled(LOG_SYSTEM_GPS) == 0);
        LOG_enable_systems(LOG_SYSTEM_GPS);
    }


    // Test case 2: Toggle LOG_SYSTEM_GPS and LOG_SYSTEM_OBC status
    
    uint8_t gps_status_original = LOG_system_logging_is_enabled(LOG_SYSTEM_GPS);
    uint8_t obc_status_original = LOG_system_logging_is_enabled(LOG_SYSTEM_OBC);
    // Toggle on, then off, then on, then back to original states, to ensure a
    // toggle has happened irrespective of the original states
    LOG_enable_systems(LOG_SYSTEM_GPS | LOG_SYSTEM_OBC);
    TEST_ASSERT(LOG_system_logging_is_enabled(LOG_SYSTEM_GPS) == 1);
    TEST_ASSERT(LOG_system_logging_is_enabled(LOG_SYSTEM_OBC) == 1);
    LOG_disable_systems(LOG_SYSTEM_GPS | LOG_SYSTEM_OBC);
    TEST_ASSERT(LOG_system_logging_is_enabled(LOG_SYSTEM_GPS) == 0);
    TEST_ASSERT(LOG_system_logging_is_enabled(LOG_SYSTEM_OBC) == 0);
    // Restore original states
    if (gps_status_original == 1) {
        LOG_enable_systems(LOG_SYSTEM_GPS);
    }
    if (obc_status_original == 1) {
        LOG_enable_systems(LOG_SYSTEM_OBC);
    }

    return 0;
}

uint8_t TEST_EXEC__LOG_all_channels_except(void)
{
    return 0;
    // Test case 1: all channels except the UHF radio
    TEST_ASSERT(LOG_all_channels_except(LOG_CHANNEL_UHF_RADIO) == (LOG_CHANNEL_ALL & ~LOG_CHANNEL_UHF_RADIO));
    // Test case 2: all channels except the umbilical UART
    TEST_ASSERT(LOG_all_channels_except(LOG_CHANNEL_UMBILICAL_UART) == (LOG_CHANNEL_ALL & ~LOG_CHANNEL_UMBILICAL_UART));
    // Test case 3: all channels except the filesystem
    TEST_ASSERT(LOG_all_channels_except(LOG_CHANNEL_FILE) == (LOG_CHANNEL_ALL & ~LOG_CHANNEL_FILE));
    // Test case 4: no channels
    TEST_ASSERT(LOG_all_channels_except(LOG_CHANNEL_ALL) == LOG_CHANNEL_NONE);

    return 0;
}


uint8_t TEST_EXEC__LOG_message(void)
{
    return 0;
    // Test case 1: Display a log message on the umbilical channel: requires interactive assessment! 
    LOG_message(LOG_SYSTEM_UNIT_TEST, 
            LOG_SEVERITY_NORMAL, 
            LOG_CHANNEL_UMBILICAL_UART, 
            "DO NOT IGNORE: This message should start with a timestamp \
followed by [UNIT_TEST:NORMAL]: DO NOT...");

    return 0;
}

