#ifndef INCLUDE_GUARD__AGENDA_FROM_FILE_H
#define INCLUDE_GUARD__AGENDA_FROM_FILE_H

#include <stdint.h>

#include "littlefs/littlefs_constants.h"

extern char *TCMD_active_agenda_filename_disabled_sentinel;
extern char TCMD_active_agenda_filename[LFS_MAX_PATH_LENGTH];

uint8_t TCMD_parse_tcmds_from_file_and_enqueue(
    const char *file_path,
    uint64_t min_tsexec_inclusive, uint64_t max_tsexec_exclusive,
    uint16_t max_enqueue_count
);

#endif // INCLUDE_GUARD__AGENDA_FROM_FILE_H
