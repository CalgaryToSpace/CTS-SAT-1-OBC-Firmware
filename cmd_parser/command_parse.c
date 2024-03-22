#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "command_parse.h"
#include "commands.h"
#include "agenda.h"

#if __linux__ == 1
#include <stdio.h>
#endif

const char PREAMBLE[] = "CTS1+";
const char WHITESPACE[] = " \t\n\r";
const char CTRL_CHAR[] = "+(,)@";
const char CRC_SPECIFIER = '=';

// Temporarily stores the error number that a function returns
// FIXME; NOT THREAD SAFE!
int err;

int main(int argc, char *argv[]) {
	if (argc < 2) return 1;

	for (int i=1; i<argc; i++) {
		command_t command;
		if ((err = parse_command(argv[i], &command)) != 0) {
			fprintf(stderr, "Error # %d\n", err);
			return err;
		}
		print_command(&command);
		// puts("\nexecuting...\n");
		// execute_command(&command);
		add_cmd_to_agenda(command_agenda, &command);
		puts("-------\n");
	}
	uint64_t mask;
	select_cmds_for_exec(&mask, command_agenda, epoch_time_ms());
    printf("0x%016llX\n", (unsigned long long)mask);

	int cmd_index;
	do {
		char cmd_result[MAX_RESULT_LEN];
		int ret = apply_cmd_mask(cmd_result, &cmd_index, command_agenda, &mask);
		if (ret != 0) return 0;
	} while (cmd_index >= 0);
	
	return 0;
}

/**
 * Take in a command string and call function associated
CTS1+<TCMD>(<param1>,<param2>)[@(<TIMESTAMP_TO_EXECUTE_AT_IN_MS>)]=[<CRC32>]<NULL>
*/
int parse_command(const char message[], command_t *cmd)
{

	// TODO: Return error messages
	char _[MAX_MSG_LEN];
	char *msg = _;
	strcpy(msg, message);
	memset(cmd, 0, sizeof(command_t));

	char *crc;
	if ((crc = strchr(msg, CRC_SPECIFIER))) {
		*(crc++) = '\0';
		char *end = strpbrk(crc, WHITESPACE);
		if (end) *end = '\0';
		cmd->crc = parse_CRC(crc);
		if (!check_CRC(msg, cmd->crc)) return E_CRC;
	}

	if ((err = remove_whitespace(msg)) != 0) return err;
// fprintf(stderr, "%zd\n", sizeof(PREAMBLE));
	if (strncmp(msg, PREAMBLE, sizeof(PREAMBLE)-1) !=  0) return E_MALFORMED;
	msg += sizeof(PREAMBLE)-1;

	char *next;
	// Parse command
	next = strchr(msg, '(');
	if (!next) return E_MALFORMED;
	*next = '\0';

	int ret = lookup_command(&cmd->cmd_code, msg);
	if (ret !=0) return ret;
	msg = next + 1;

	// Parse arguments
	bool quote = false;
	while ((next = strpbrk(++next, "\",)"))) {
		if (*next == '"') {
			quote = !quote;
			continue;
		}
		if (*next == ')') {
			if (quote) continue;  // Ignore ')' inside quotes
			*next = '\0';
			if (next > msg) cmd->nargs++;
			break;
		}
		if (*next == ',') {
			if (quote) continue;  // Ignore ',' inside quotes
			*next = '\0';  // Separate args into separate strings
			cmd->nargs++;
		}
	}
	if (!next) return E_MALFORMED;

	size_t arg_len = next-msg+1;
	if (arg_len > MAX_ARG_LEN) return E_OVERFLOW;
	memcpy(cmd->args, msg, arg_len);  // Copy the whole arg list
	msg = next + 1;

	// Parse timestamp
	if (msg[0] == '@' && msg[1] == '(') {
		msg += 2;
		next = strchr(msg, ')');
        if (!next) return E_MALFORMED;
        *next = '\0';
		char *endptr;
		cmd->timestamp = strtoull(msg, &endptr, 10);
		if (cmd->timestamp == 0 && errno == ERANGE) {
			errno = 0;
			return E_TIME_FMT;
		}
		msg = next;
	}

	return 0;
}

/**
 * Remove whitespaces from between sections of msg.
 * If a whitespace appears inside a clause, exit with error.
 * Leave whitespaces inside double quotes.
 * return 0 on success.
 * on failure, return the negative index of where the error occurred.
*/
int remove_whitespace(char message[])
{
	if (!message[0]) return E_MALFORMED;
	char *curr = message, *tail = message;
	bool quote = false;
	
	// If first char is whitespace
	if (strchr(WHITESPACE, curr[0]))
		*(curr++) = '+';  // Add a dummy ctrl char to the slot
		// Advance curr but not tail
	do {
		if (*curr == '"') {
			quote = !quote;
		// If this char is whitespace (skip if we are between quotes)
		} else if (!quote && strchr(WHITESPACE, *curr)) {
			// If previous char is ctrl (don't look back if it's the first char)
			if (strchr(CTRL_CHAR, curr[-1])) {
				// Extend the ctrl char so that it can be seen as previous later
				*curr = curr[-1];
				continue;
			}
			// If next char is ctrl or whitespace
			if (strchr(CTRL_CHAR, curr[1]) || strchr(WHITESPACE, curr[1]))
				continue;

			fprintf(stderr, "Error at index: %to\n", curr-message);
			return E_MALFORMED;
		}
		// Shift the current char into the previous empty space if there is one
		if (tail < curr)
			*tail = *curr;

		++tail;
	} while (*(++curr));

	*tail = '\0';
	return 0;
}

bool check_CRC(const char message[], const uint64_t crc)
{
	// TODO: implement
	// fprintf(stderr, "%llu\n", (long long unsigned int)crc);
	return true;
}

/**
 * Take in a CRC32 in hex representation and convert it to uint64_t
 * Return 0 if failed
*/
uint64_t parse_CRC(const char crc_hex[])
{
	char *end;
	uint64_t crc = (uint64_t) strtoull(crc_hex, &end, 16);
	if (end != strchr(crc_hex, '\0')) return 0;
	return crc;
}


int execute_command(char *result, const command_t *cmd)
{
    return command_list[cmd->cmd_code].fn(result, cmd->nargs, cmd->args);
}


#if __linux__ == 1

void print_command(const command_t *cmd)
{
	printf("CRC: %x\n", cmd->crc);
	printf("cmd_code: 0x%02X\n", cmd->cmd_code);
	printf("nargs: %d\n", cmd->nargs);
	printf("timestamp: %ld\n", cmd->timestamp);
	print_args(cmd->nargs, cmd->args);
}

int print_args(uint8_t nargs, const char *args)
{
    const char *arg_list[nargs];
    unpack_args((const char**) &arg_list, nargs, args);

    for (int i=0; i<nargs; i++)
        printf("arg %d: %s\n", i, arg_list[i]);
    return 0;
}

/**
 * Executes one command specified in the mask.
 * Removes the command executed from the mask.
 * Returns the result of the command and the index of the command executed.
 * Does not modify the bitmask 
*/
int apply_cmd_mask(char *result, int *cmd_idx, const command_t agenda[], uint64_t *mask)
{
	*cmd_idx = -1;
	int index = __builtin_ffsll(*mask)-1;  // Find occupied slot
	int ret = execute_command(result, &agenda[index]);
	if (ret != 0) return ret;
	*cmd_idx = index;
	remove_cmd_from_bitmap(index, mask);
	return 0;
}

#endif