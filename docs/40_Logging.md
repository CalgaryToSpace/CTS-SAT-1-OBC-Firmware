# Logging

Satellites do things both as a result of us earthlings sending telecommands, as a result of scheduled telecommands, and as a result of other autonomous actions (e.g., automatically turning of subsystems when the power gets low).

Nearly everything the satellite does must be logged so that it can be audited in the case of problems.

## Logging in Embedded Systems

In general embedded systems development, you commonly print data to a UART terminal for debugging purposes. The `DEBUG_uart_print_str(...)` function is used for this purpose.

This is a simple and effective way to debug code, but it is not a good way to log data in a satellite. We can't plug a UART cable into a satellite flying through space at 8 km/s.

## Logging in Space

The logging system on this satellite is rather simple: call the `LOG_message(...)` function with the message you want, and it will be logged to a file, over the radio, to a file, to memory, and to the UART debug console (if connected on earth).

## How to use `LOG_message(...)`

YOU, a developer, must be use the `LOG_message(...)` function. Here's how:

1. In just about every `.c` file, include: `#include "log/log.h"`
2. Call `LOG_message(...)` with the message you want to log with the relevant subsystem, log severity, which sinks (`LOG_SINK_ALL` by default), a message, and any printf-like arguments for format strings in the message:
```c
LOG_message(
    LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
    "Opened file to read: %s",
    file_name
);
```

## `LOG_message(...)` Args:

The following are very small and specific details about the arguments to `LOG_message(...)`.

### Subsystems

There are several subsystems, defined in an enum. Use the right one.

### Log Levels

There are several log levels, defined in an enum.

Sometimes, stuff works (use `LOG_SEVERITY_NORMAL`). Sometimes it doesn't (use `LOG_SEVERITY_WARNING` or `LOG_SEVERITY_ERROR`). Sometimes, the thing not working can cause other things to not work (use `LOG_SEVERITY_CRITICAL`).

### Sinks

There are several sinks. Generally, all log messages should go to all sinks (`LOG_SINK_ALL`).

Sometimes, however, there may be a time where you want to exclude a specific sink. For example, if you are logging a message saying "using the filesystem failed", you probably don't want to log that message to the failing filesystem. In such a case, use `LOG_all_sinks_except(LOG_SINK_FILE)`.

Several sinks can chained together with the bitwise OR operator (`|`). For example, to log to the UART and the radio, use `LOG_SINK_UART | LOG_SINK_RADIO`. To log to everything except the filesystem and radio, use `LOG_all_sinks_except(LOG_SINK_FILE | LOG_SINK_RADIO)`.
