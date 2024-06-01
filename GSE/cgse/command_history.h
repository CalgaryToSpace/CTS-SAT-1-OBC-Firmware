#ifndef __INCLUDE_GUARD__COMMAND_HISTORY_H_
#define __INCLUDE_GUARD__COMMAND_HISTORY_H_

#include <stddef.h>

int CGSE_store_command(char *cmd);
int CGSE_remove_command(size_t index);
char * CGSE_recall_command(size_t index);
size_t CGSE_number_of_stored_commands(void);


#endif // __INCLUDE_GUARD__COMMAND_HISTORY_H_
