# C General Guidelines
These are general guidelines all the C code should follow within this repository.

## Naming Convention
* Variable names should be snake case (e.g., `packet_buffer`, not `packetBuffer`).
* Cross-file variable/function names should the namespace conventions in:  `docs/C_Namespaces.md`
    * E.g., all EPS-related functions should start with `EPS_`.
* Include guards should follow the style `__INCLUDE_GUARD__FILE_NAME_H__`.

## Best Coding Practices
* Please use braces around all `if/else/while/for`, even if the body is only a single line.
* Includes should go in `.c` files and not in `.h` files, if possible. This limits the scope of the functions.
* For all systems, we must differentiate at least between functions which are meant to be used _only internally_ to that system (e.g., `FLASH_activate_chip_select()`) and functions which are meant to be used from _anywhere in the codebase_  (e.g., `LFS_read_file_from_filesystem()`). They must go in separate files (e.g., named `eps_drivers.c`/`eps_internal_drivers.c` for internal functions; and `eps_helpers.c`/`eps_commands.c`/`eps_datatypes.h` for external functions).
* Do not commit changes to `main.c` to git, ever. Instead, create a file called `your_subsystem_testing.c` in the folder with the rest of the code you're working on, if you need to use test code. You can call that code from a telecommand or from a *single line* in main.c.
* Everything must be allocated on the stack! No heap allocation. No using `free()`. No using `malloc()`.
* If you create a new telecommand parsing related function, you must create unit tests for it.
* No null pointers. No void pointers.

## Error Handling
* When a function runs successfully, it should return `0`. If it errors, it should return a non-zero value.
* The return value from all functions **must** be used/checked. Errors must be propagated up the call stack. All HAL functions must have their return values checked.
    * Consider whether each error is fatal (e.g., "failed to mount filesystem"), vs. non-fatal (e.g., "tried to mount the filesystem, but it's already mounted").

## Documentation
* Please use the comment prefixes `// TODO:` and `// FIXME:` to denote things you need to come back to/check on later.
* Functions must have docstrings, like this:

```c
/// @brief Briefly describe what the function does here.
/// @param[in] input1 Description of the first input parameter.
/// @param[in] input2 Description of the second input parameter.
/// @param[out] output_arr Description of an output parameter, which gets written to.
/// @param[in] output_arr_len Length of the output buffer, for example.
/// @return 0 if successful, non-zero if an error occurred.
uint8_t function_name(int input1, int input2, int *output_arr, int output_arr_len) {
    // Function body here.
}
```

For telecommands executors (`TCMDEXEC_` functions), the docstring should be formatted to describle each argument, in this exact format. For example:
```c

/// @brief Telecommand: Write data to a file in LittleFS
/// @param args_str
/// - Arg 0: File name
/// - Arg 1: String to write to file
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_fs_write_file(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                               char *response_output_buf, uint16_t response_output_buf_len) {
    char arg_file_name[64] = {0};
    char arg_file_content[100] = {0};
    const uint8_t parse_file_name_result = TCMD_extract_string_arg(args_str, 0, arg_file_name, sizeof(arg_file_name));
    const uint8_t parse_file_content_result = TCMD_extract_string_arg(args_str, 1, arg_file_content, sizeof(arg_file_content));
}
```
