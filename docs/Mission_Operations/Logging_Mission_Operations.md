# Logging Mission Operations Guide

## Log Sinks

Logs go to the following locations:
* Umbilical UART (useful for ground testing only, of course)
* UHF Radio (disabled by default)
* Files in the `/logs/<timestamp>.log` folder.

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
