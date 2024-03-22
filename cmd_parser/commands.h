#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_RESULT_LEN 512

#define NUM_COMMANDS 3
// **** COMMAND DEFINITIONS **** //
#define CMD_0x00 0x00
#define CMD_NOOP 0x01
#define CMD_PRINT 0x02

// Returns status code, takes pointer to output buffer, num of args, null-separated arg string
typedef int (*command_function_t)(char*, uint8_t, const char*);
typedef uint8_t command_code_t;

typedef struct {
    command_code_t code;
    command_function_t fn;
    char *name;
} command_list_entry_t;

extern const command_list_entry_t command_list[NUM_COMMANDS];

void unpack_args(const char *ptrs[], uint8_t nargs, const char *args);
int lookup_command(command_code_t *cmd_code, const char *cmd_str);

// COMMANDS
int cmd_noop(char *result, uint8_t nargs, const char *args);
int cmd_print(char *result, uint8_t nargs, const char *args);


#endif  // COMMANDS_H