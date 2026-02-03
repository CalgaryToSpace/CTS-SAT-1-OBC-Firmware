#include <stdint.h>

#include "log/log_a_logging_error.h"

#include "log/log.h"
#include "log/log_sinks.h"
#include "log/lazy_file_log_sink.h"

/// @brief Log an error which occurs while writing to the file log sink.
/// @param message Message explaining the error.
/// @note Why? We can't use the normal LOG_message function from within log
///     sinks, because the log message gets stored into a temporary log buffer.
void LOG_log_a_logging_error_if_file_is_broken(const char message[]) {
    // File logging is broken. Log to everything not a file!
    LOG_to_umbilical_uart("\n");
    LOG_to_umbilical_uart(message);
    LOG_to_umbilical_uart("\n");

    LOG_to_uhf_radio(message);

    // FIXME(Issue #398): log to memory buffer
}
