/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */
#ifndef GS_CSP_EXTERNAL_CSP_DEBUG_H
#define GS_CSP_EXTERNAL_CSP_DEBUG_H

/**
   @file

   Use GomSpace log system instead of standard libcsp debug/log.

   @note Not part of standard libcsp.
*/

#include <gs/util/log/log.h>

#ifdef __cplusplus
extern "C" {
#endif

/** All libcsp logs is logged on group \a csp */
GS_LOG_GROUP_EXTERN(gs_csp_log);

/** Log error -> GomSpace log \a error on group \a csp */
#define csp_log_error(...)     log_error_group(gs_csp_log, ##__VA_ARGS__)
/** Log warning -> GomSpace log \a warning on group \a csp */
#define csp_log_warn(...)      log_warning_group(gs_csp_log, ##__VA_ARGS__)
/** Log info -> GomSpace log \a info on group \a csp */
#define csp_log_info(...)      log_info_group(gs_csp_log, ##__VA_ARGS__)
/** Log packet -> GomSpace log \a info on group \a csp */
#define csp_log_packet(...)    log_info_group(gs_csp_log, ##__VA_ARGS__)
/** Log buffer -> GomSpace log \a trace on group \a csp */
#define csp_log_buffer(...)    log_trace_group(gs_csp_log, ##__VA_ARGS__)
/** Log protocol -> GomSpace log \a debug on group \a csp */
#define csp_log_protocol(...)  log_debug_group(gs_csp_log, ##__VA_ARGS__)
/** Log lock -> GomSpace log \a trace on group \a csp */
#define csp_log_lock(...)      log_trace_group(gs_csp_log, ##__VA_ARGS__)

/** Toogle debug leve -> not used */
#define csp_debug_toggle_level(...)  {}
/** Set debug level -> not used */
#define csp_debug_set_level(...)     {}
/** Get debug level -> not used */
#define csp_debug_get_level(...)     {}
    
#ifdef __cplusplus
}
#endif
#endif
