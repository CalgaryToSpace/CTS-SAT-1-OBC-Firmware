#include "telecommand_exec/agenda_from_file.h"

#include "littlefs/littlefs_helper.h"

/// @brief The file path of the agenda file to load upcoming telecommands from.
/// @warning While a default agenda is set, it is critical to understand that you SHOULD NOT
///     fill this agenda file unless absolutely necessary. If you create and fill an agenda
///     with this default name, it will run on every boot, thus defeating the purpose of
///     reboots being a stable, fail-safe startup mode.
/// @note If system stability ends up being super bad, you can put commands in the default to run at
///     the earth poles on subsequent reboots. That's probably the only time you should ever
///     create this file though!
/// @note Nominally, put the agenda in another random file, then update this config string to point
///     to that other file. That way, on reboot, the agenda stops running (as a fail-safe)!
char TCMD_active_agenda_filename[LFS_MAX_PATH_LENGTH] = "default_tcmd_agenda.txt";
