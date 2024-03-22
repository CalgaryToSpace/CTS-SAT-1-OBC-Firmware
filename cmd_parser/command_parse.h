#ifndef COMMAND_PARSE_H
#define COMMAND_PARSE_H

#include <stdint.h>
#include <stdbool.h>
#include "commands.h"

#define MAX_MSG_LEN 512
#define MAX_ARG_LEN 64
#define MAX_ERR_MSG_LEN 128

// TODO make these fixed-width ints
// Negative codes are for parser errors, positive are returned by the commands
typedef enum {
    E_MALFORMED = -1,
    E_CRC = -2,
    E_TIME_FMT = -3,
    E_NOT_FOUND = -4,
    E_OVERFLOW = -5,
    E_OOM = -6,
    E_NULL = -7,
    E_UNKNOWN = -255
} errcode_t;

// TODO: Byte alignment?
typedef struct {
    uint64_t timestamp;
    uint32_t crc;  // Possibly store this to check immediately before execution
    uint8_t nargs;  // Number of args
    char args[MAX_ARG_LEN];  //  Null-separated list of args
    command_code_t cmd_code;
} command_t; 



// // TODO: Byte alignment?
// typedef struct {
//     errcode_t err_code;
//     char message[MAX_ERR_MSG_LEN];
// } error_t;
//
// typedef struct {
//     union {
//         command_t;
//         error_t;
//     };
// } command_or_error_t;


int parse_command(const char message[], command_t *command);
int execute_command(char *result, const command_t *cmd);
int remove_whitespace(char message[]);
bool check_CRC(const char message[], const uint64_t crc);
uint64_t parse_CRC(const char crc_hex[]);
int apply_cmd_mask(char *result, int *cmd_idx, const command_t agenda[], uint64_t *mask);


#if __linux__ == 1

void print_command(const command_t *cmd);
int print_args(uint8_t nargs, const char *args);

#else

#define print_command(...)
#define print_args(...)
#define printf(...)
#define fprintf(...)
#define puts(...)

#endif

#endif  // COMMAND_PARSE_H