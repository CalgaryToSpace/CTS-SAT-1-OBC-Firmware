# Logging Mission Operations Guide

## Log Sinks

Logs can go to the following locations ("sinks"):
* `LOG_SINK_UHF_RADIO = 1`: UHF Radio (disabled by default)
* `LOG_SINK_FILE = 2`: Files in the `/logs/<timestamp>.log` folder (disabled by default).
* `LOG_SINK_UMBILICAL_UART = 4`: Umbilical UART (useful for ground testing only, of course)

In deployment, "logging" is disabled by default. It is recommended to enable radio logging at the start of a pass, and disable it at the end of a pass.

## Logging to File

Logging to file is less well-tested than most satellite features. By default, to be in a "fail-safe state", logging to file is disabled.

Do not fear enabling it, but understand that you may experience more system restarts than normal, and certain features may be slower/broken with logging-to-file enabled.

### Procedure: Enable Logging to File

1. Enable the FILE (2) log sink: `CTS1+log_set_sink_enabled_state(2,1)!`
2. For each `LOG_SYSTEM` you want enabled logs, enable file logging: `CTS1+log_set_system_file_logging_enabled_state(32,1)!`
3. You will see timestamped files appear in the `/logs/` folder.

A new log file is started periodically, based on configuration variable `LOG_file_rotation_interval_sec`.


### Behaviour Notes

* Logs are not immediately written to the file, but rather are buffered and then written to the file periodically, based on the `LOG_file_flush_interval_sec` configuration variable. You won't see a file appear immediately after enabling file logging.


## Logging to Radio

By default, the radio log sink is disabled.

### Procedure: Enable Radio Logs

This procedure is recommended at the start of passes.

1. `CTS1+log_set_sink_enabled_state(1,1)!`

### Procedure: Disable Radio Logs

This procedure is recommended at the end of passes, and right before bulk data transfers.

1. `CTS1+log_set_sink_enabled_state(1,0)!`

## Debug Logs

By default, debug logs are turned off. Debug logs are any log message that the developer(s) decided are mostly only useful under off-nominal or "debugging" scenarios, and are otherwise "spam".

Debug logs are masked off for all log _sinks_, but are masked on for all log _systems_. Thus, you must only use the `log_set_sink_debugging_messages_state`, and not the `log_set_system_debugging_messages_state` command (at minimum).

### Procedure: Enable Debug Logs

1. Optional: Start from a nominal/rebooted state.
2. Enable the radio and/or file log sinks (e.g., `CTS1+log_set_sink_enabled_state(1,1)!`).
3. Enable debug mode for whichever sink is relevant: `CTS1+log_set_sink_debugging_messages_state(1,1)!`
