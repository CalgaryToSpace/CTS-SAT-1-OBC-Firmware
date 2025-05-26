#ifndef INCLUDE_GUARD__LITTLEFS_TELECOMMANDS_H
#define INCLUDE_GUARD__LITTLEFS_TELECOMMANDS_H

#include <stdint.h>

int32_t LFS_list_directory_json_dict(
    const char root_directory[],
    uint16_t offset, int16_t count,
    char *json_output_buf, uint16_t json_output_buf_size
);


#endif // INCLUDE_GUARD__LITTLEFS_TELECOMMANDS_H
