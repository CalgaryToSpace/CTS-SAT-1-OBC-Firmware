#include "unit_tests/unit_test_helpers.h"
#include "log/log.h"

uint8_t TEST_EXEC__LOG_enable_disable_LOG_SINK_FILE(void)
{
    uint8_t initial_is_sink_enabled = LOG_is_sink_enabled(LOG_SINK_FILE);

    // Test case 1: enable and disable logging to LOG_SINK_FILE 
    // Enable
    LOG_set_sink_enabled_state(LOG_SINK_FILE, LOG_SINK_ON);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_FILE) == 1);
    // Disable
    LOG_set_sink_enabled_state(LOG_SINK_FILE, LOG_SINK_OFF);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_FILE) == 0);
    // Restore initial state
    LOG_set_sink_enabled_state(LOG_SINK_FILE, initial_is_sink_enabled);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_FILE) == initial_is_sink_enabled);

    return 0;
}

uint8_t TEST_EXEC__LOG_enable_disable_LOG_SINK_UHF_RADIO(void)
{
    uint8_t initial_is_sink_enabled = LOG_is_sink_enabled(LOG_SINK_UHF_RADIO);

    // Test case 1: enable and disable logging to LOG_SINK_UHF_RADIO
    // Enable
    LOG_set_sink_enabled_state(LOG_SINK_UHF_RADIO, LOG_SINK_ON);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_UHF_RADIO) == 1);
    // Disable
    LOG_set_sink_enabled_state(LOG_SINK_UHF_RADIO, LOG_SINK_OFF);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_UHF_RADIO) == 0);
    // Restore initial state
    LOG_set_sink_enabled_state(LOG_SINK_UHF_RADIO, initial_is_sink_enabled);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_UHF_RADIO) == initial_is_sink_enabled);

    return 0;
}

uint8_t TEST_EXEC__LOG_enable_disable_LOG_SINK_UMBILICAL_UART(void)
{
    uint8_t initial_is_sink_enabled = LOG_is_sink_enabled(LOG_SINK_UMBILICAL_UART);

    // Test case 1: enable and disable logging to LOG_SINK_UMBILICAL_UART
    // Enable
    LOG_set_sink_enabled_state(LOG_SINK_UMBILICAL_UART, LOG_SINK_ON);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_UMBILICAL_UART) == 1);
    // Disable
    LOG_set_sink_enabled_state(LOG_SINK_UMBILICAL_UART, LOG_SINK_OFF);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_UMBILICAL_UART) == 0);
    // Restore initial state
    LOG_set_sink_enabled_state(LOG_SINK_UMBILICAL_UART, initial_is_sink_enabled);
    TEST_ASSERT(LOG_is_sink_enabled(LOG_SINK_UMBILICAL_UART) == initial_is_sink_enabled);

    return 0;
}

uint8_t TEST_EXEC__LOG_enable_disable_LOG_FILE_LOGGING_FOR_SYSTEM_MPI(void)
{
    uint8_t initial_is_file_logging_enabled = LOG_is_system_file_logging_enabled(LOG_SYSTEM_MPI);

    // Test case 1: enable and disable logging to LOG_SYSTEM_MPI
    // Enable
    LOG_set_system_file_logging_enabled_state(LOG_SYSTEM_MPI, LOG_SYSTEM_ON);
    TEST_ASSERT(LOG_is_system_file_logging_enabled(LOG_SYSTEM_MPI) == 1);
    // Disable
    LOG_set_system_file_logging_enabled_state(LOG_SYSTEM_MPI, LOG_SYSTEM_OFF);
    TEST_ASSERT(LOG_is_system_file_logging_enabled(LOG_SYSTEM_MPI) == 0);
    // Restore initial state
    LOG_set_system_file_logging_enabled_state(LOG_SYSTEM_MPI, initial_is_file_logging_enabled);
    TEST_ASSERT(LOG_is_system_file_logging_enabled(LOG_SYSTEM_MPI) == initial_is_file_logging_enabled);

    return 0;
}

uint8_t TEST_EXEC__LOG_all_sinks_except(void)
{
    // Test case 1: all sinks except the UHF radio
    TEST_ASSERT(LOG_all_sinks_except(LOG_SINK_UHF_RADIO) == (LOG_SINK_ALL & ~LOG_SINK_UHF_RADIO));
    // Test case 2: all sinks except the umbilical UART
    TEST_ASSERT(LOG_all_sinks_except(LOG_SINK_UMBILICAL_UART) == (LOG_SINK_ALL & ~LOG_SINK_UMBILICAL_UART));
    // Test case 3: all sinks except the filesystem
    TEST_ASSERT(LOG_all_sinks_except(LOG_SINK_FILE) == (LOG_SINK_ALL & ~LOG_SINK_FILE));
    // Test case 4: all sinks except the filesystem and umbilical UART
    TEST_ASSERT(LOG_all_sinks_except(LOG_SINK_FILE | LOG_SINK_UMBILICAL_UART) == (LOG_SINK_ALL & ~(LOG_SINK_FILE | LOG_SINK_UMBILICAL_UART)));
    // Test case 5: no sinks
    TEST_ASSERT(LOG_all_sinks_except(LOG_SINK_ALL) == LOG_SINK_NONE);

    return 0;
}

