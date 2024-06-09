# C General Guidelines
These are general guidelines all the C code should be following within this repository.

## Naming Convention
* Variable names should be snake case (e.g., `packet_buffer`, not `packetBuffer`).
* ... More information can be found in `docs\C_Namespaces.md`

## Best Coding Practices
* Please use braces around all `if/else/while/for`, even if the body is only a single line.
* Includes should go in `.c` files and not in `.h` files, if possible. This limits the scope of the functions.
* For all systems, we must differentiate at least between functions which are meant to be used _only internally_ to that system (e.g., `activate_chip_select()`) and functions which are meant to be used from _anywhere in the codebase_  (e.g., `read_file_from_filesystem()`). They must go in separate files (e.g., named "eps_drivers"/"eps_internal_drivers" for internal functions; and "eps_helpers"/"eps_commands"/"eps_datatypes" for external functions).
* Do not commit changes to `main.c` to git, ever. Instead, create a file called `your_subsystem_testing.c` in the folder with the rest of the code you're working on, if you need to use test code. You can tell that code from a telecommand or from a *single line* in main.c.
* Everything must be allocated on the stack! No heap allocation. No using `free()`. No using `malloc()`.
* If you create a new telecommand parsing related function, you must create unit tests for it.

## Error Handling
* The return value from all functions **must** be used/checked. Errors must be propagated up the call stack. All HAL functions must have their return values checked.
* Error return values should generally be negative numbers, if the function ran as intended, it should return 0.

## Documentation
* Please use the comment prefixes `// TODO:` and `// FIXME:` to denote things you need to come back to/check on later.
* User created functions must have docstrings which contain `@brief` - giving an overview of the function, `@param` - how parameters are used in the function, and `@return` - expected return values and their indication.