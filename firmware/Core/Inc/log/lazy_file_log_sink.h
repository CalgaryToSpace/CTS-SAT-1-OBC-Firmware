#ifndef INCLUDE_GUARD__LAZY_FILE_LOG_SINK_H
#define INCLUDE_GUARD__LAZY_FILE_LOG_SINK_H

#include <stdint.h>

#include "littlefs/lfs.h"

void LOG_to_file_lazy(const char filename[], const char msg[]);

void LOG_subtask_handle_sync_and_close_of_current_log_file();

#endif // INCLUDE_GUARD__LAZY_FILE_LOG_SINK_H
