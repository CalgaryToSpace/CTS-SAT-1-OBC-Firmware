# Telecommand Specification

The following specification defines how telecommands are constructed to control the satellite.

The same specification is used whether the commands are delivered over Debug UART or Radio.

## Examples

```
CTS1+hello_world()!
CTS1+run_all_unit_tests()!
CTS1+echo_back_args(arg1,arg2,arg3)!
CTS1+echo_back_uint32_args(1234,invalid_arg,32)!
CTS1+echo_back_args(arg1,arg2,arg3)@tssent=1716611908453@tsexec=1716611999999!
CTS1+echo_back_args(arg1,arg2,arg3)@tssent=1716611908453@tsexec=1716611999999#sha256=XXxxxXXXxx+xx/xx===!
```

## Command Format

Every command contains the following subsections, concatenated together with no delimeters:

1. Target Device Prefix: `CTS1+` (meaning CTS-SAT-1)
2. Command Name: e.g., `hello_world`
3. Arguments: e.g., `(arg1,arg2,arg3)` (no spaces between arguments allowed).
    * For no arguments, send `()`.
    * All arguments are positional (order matters).
    * All arguments are required (no optional arguments).
4. Command Suffix Tag 1 - Timestamp Sent: `@tssent=xxx`, where `xxx` is the UInt64 unix timestamp (milliseconds) in base10 when the command was sent.
    * Why? Each command must be sent at a unique timestamp. On the satellite, we avoid double-executing a command that is sent twice by tracking the distinct list of `tssent` values received.
    * Commands with duplicate/repeated `tssent` values will be ignored, allowing you to uplink the same telecommand multiple times without executing it multiple times.
5. Command Suffix Tag 2 - Delayed Execution Timestamp: `@tsexec=xxx`, where `xxx` is the UInt64 unix timestamp (milliseconds) in base10 when the command should be executed, or 0 to execute immediately. Absence of this tag means to execute immediately.
    * Why? This allows for scheduling commands in the future.
6. Command Suffix Tag 4 - Log Filename: `@log_filename=xxxx.txt`, where `xxxx` is the name of the log file to store the telecommand response in.
    * Why? This allows for storing the telecommand response in a file (e.g., for storing power data during science data collection).
    * Max length is 31 characters (32 bytes including null terminator).
7. 🚧 Command Suffix Tag 3 - Authenticated SHA256 Hash: `#sha256=xxx`, where `xxx` is the 32-byte base64 representation of the sha256 hash of all characters preceding, concatenated with the secure key used to authenticate communications with the satellite.
    * Why? This ensures that the command was not malformed in transit, and is a way of validating that the command was sent by the CTS team and not a malicious actor.
8. End-of-Command Marker: `!`
    * Why? To ensure that the command is complete and not truncated. Also, to ensure that the string-being-parsed does not have multiple commands concatenated together.
9. Extra trailing characters/whitespaces are not required, but are permitted.
    * Why? Serial terminals often add extra characters.

Note: The 🚧 emoji indicates that the command part is planned, but is not yet implemented.

## Notes

The following characters have special meanings, and cannot be used in any variable parts of the command:

```
+ ! @ # ( ) , = :
```
