# Telecommand Feature List

Helpful tips/reminders of telecommand features and usage.

## Overview

* Telecommands have a main response (null-terminated string) and a response code (error code, or 0 for success).
* Telecommands can send log messages while being executed.
    * By default, log messages are sent to the filesystem, the radio, downlinked, and cached in memory.
* Telecommands can be executed right away, or scheduled in the agenda for later.
* Telecommands are denoted by a unique ID (tssent) which is a 64-bit integer.
    * Commands with duplicate/repeated `tssent` values will be ignored, allowing you to uplink the same telecommand multiple times without executing it multiple times.
* Send the `@log_filename=xxxx.txt` suffix tag to store the telecommand response in a file.
    * This is especially helpful for storing power data during science data collection, for example.
