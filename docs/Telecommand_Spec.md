# Telecommand Specification

The following specification defines how telecommands are constructed to control the satellite.

The same specification is used whether the commands are delivered over Debug UART or Radio.

## Command Format

Every command contains the following subsections, concatenated together with no delimeters:

1. Target Device Prefix: `CTS1+` (meaning CTS-SAT-1)
2. Command Name: e.g., `hello_world`
3. Arguments: e.g., `(arg1,arg2,arg3)` (no spaces between arguments allowed).
    * For no arguments, send `()`.
    * All arguments are positional (order matters).
    * All arguments are required (no optional arguments).
4. 🚧 Command Suffix Tag 1 - Timestamp Sent: `@tssent=xxx`, where `xxx` is the UInt64 value (milliseconds) in base10 when the command was sent.
5. 🚧 Command Suffix Tag 2 - Delayed Execution Timestamp: `@tsexec=xxx`, where `xxx` is the UInt64 value (milliseconds) in base10 when the command should be executed, or 0 to execute immediately.
5. 🚧 Command Suffix Tag 3 - Authenticated SHA256 Hash: `#sha256=xxx`, where `xxx` is the 32-byte base64 representation of the sha256 hash of all characters preceding, concatenated with the secure key used to authenticate communications with the satellite.
6. 🚧 End-of-Command Marker: `!`
7. Extra trailing characters/whitespaces are not required, but are permitted.

NOTE: the 🚧 emoji indicates that the command part is not yet implemented

## Examples

```
CTS1+hello_world()!
CTS1+run_all_unit_tests()!
CTS1+echo_back_args(arg1,arg2,arg3)!
CTS1+echo_back_uint32_args(1234,invalid_arg,32,68,42,65989985)!
CTS1+echo_back_args(arg1,arg2,arg3)@tssent=1716611908453@tsexec=1716611999999#sha256=XXxxxXXXxx+xx/xx===!
```

```
# TODO: Update this spec as integration continues. This spec isn't fully implemented yet, and is subject to change.
```
