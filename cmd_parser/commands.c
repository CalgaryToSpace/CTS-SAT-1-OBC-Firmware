#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if __linux__ == 1
#include <stdio.h>
#endif

#include "commands.h"
#include "command_parse.h"

const command_list_entry_t command_list[NUM_COMMANDS] = {
    {CMD_0x00, cmd_noop, ""},
    {CMD_NOOP, cmd_noop, "NOOP"},
    {CMD_PRINT, cmd_print, "PRINT"}
};

/**
 * Parse a command string and return the corresponding command code
*/
int lookup_command(command_code_t *cmd_code, const char *cmd_str)
{
    // fprintf(stderr, "%s\n", cmd_str);
    for (int i=0; i<NUM_COMMANDS; i++) {
        // fprintf(stderr, "'%s' == '%s'\n", cmd_str, command_list[i].name);
        if (strcmp(cmd_str, command_list[i].name) == 0) {
            *cmd_code = command_list[i].code;
            return 0;
        }
    }
    return E_NOT_FOUND;
}

void unpack_args(const char *ptrs[], uint8_t nargs, const char *args)
{
    if (nargs == 0) return;
    ptrs[0] = args;

    for (int i=1; i<nargs; i++)
        ptrs[i] = strchr(ptrs[i-1], '\0') + 1;
}

// COMMAND FUNCTIONS

int cmd_noop(char *result, uint8_t nargs, const char *args)
{
    return 0;
}

int cmd_print(char *result, uint8_t nargs, const char *args)
{
    puts("Executing cmd_print...");
    print_args(nargs, args);
    return 0;
}
