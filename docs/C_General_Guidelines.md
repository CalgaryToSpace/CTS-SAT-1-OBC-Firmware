# C General Guidelines

These are general guidelines all the C code should be following within this repository.

## Best Coding Practices

* Please use the comment prefixes `// TODO:` and `// FIXME:` to denote things you need to come back to/check on later.
* The return value from all functions **must** be used/checked. Errors must be propagated up the call stack. All HAL functions must have their return value checked.
* Please use braces around all `if/else/while/for`, even if the body is only a single line.
* Variable names should be snake case (e.g., `packet_buffer`, not `packetBuffer`).
* Includes should go in `.c` files and not in `.h` files, if possible. This limits the scope of the functions.
* For all systems, we must differentiate at least between functions which are meant to be used _only internally_ to that system (e.g., `activate_chip_select()`) and functions which are meant to be used from _anywhere in the codebase_  (e.g., `read_file_from_filesystem()`). They must go in separate files (e.g., named "eps_drivers"/"eps_internal_drivers" for internal functions; and "eps_helpers"/"eps_commands"/"eps_datatypes" for external functions). 