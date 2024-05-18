# Telecommand Specification

The following specification defines how telecommands are constructed to control the satellite.

The same specification is used whether the commands are delivered over UART or Radio.

## Command Format

Every command contains the following subsections, concatenated together with no delimeters:

1. Target Device: `CTS1+`
2. Command Name: e.g., `hello_world`
3. Arguments: e.g., `(arg1, arg2, arg3)`
4. Command Metadata 1 - Timestamp: `@ts=xxx`, where `xxx` is the UInt64 value in decimal
5. Command Metadata 2 - Signed SHA256 Hash: `@sha256=xxx`, where `xxx` is the 32-byte representation of the sha256 hash of all characters preceding, concatenated with the secure key used to authenticate communications with the satellite.
6. End-of-Command Marker: `!`
7. Any extra characters, whitespaces, etc. (not required, but permitted).

```
# TODO: Update this spec as integration continues. This spec isn't fully implemented yet, and is subject to change.
```
