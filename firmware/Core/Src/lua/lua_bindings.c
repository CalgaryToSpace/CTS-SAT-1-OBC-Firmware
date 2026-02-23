#include "lua/lua_bindings.h"
#include "log/log.h"
#include "littlefs/littlefs_helper.h"
#include "stdlib.h"


int l_print(lua_State *L) {
    const char* str = lua_tostring(L, 1);
    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL, "lua(print): %s", str);
    return 0;
    
}


int l_LFS_write_file(lua_State *L) {
    const char* path = lua_tostring(L, 1);
    const uint8_t* buf = lua_tostring(L, 2);

    const int32_t len = strlen(buf); 

    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL, "lua: calling LFS_write_file(%s, ...)", path);

    const int16_t res = LFS_write_file(path, buf, len);

    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL, "lua: LFS_write_file returned %d",  res);

    return 0;
}


int l_LFS_read_file(lua_State *L) {
    const char* path = lua_tostring(L, 1);
    const int32_t offset = lua_tointeger(L, 2);
    const int32_t len = lua_tointeger(L, 3);

    // Constrain for safety reasons.
    if (len > 10000){
        LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "lua: cannot read more than 10000 bytes at a time.");
        return 0;
    }

    // Could use malloc here.
    uint8_t buf[10000];
    if (buf == NULL){
        LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "lua: malloc failed.");
        return 0;
    }

    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL, "lua: calling LFS_read_file()");

    const lfs_ssize_t num_bytes = LFS_read_file(path, offset, buf, len);

    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL, "lua: LFS_read_file returned %d",  num_bytes);

    // Push the buffer onto the lua stack as a string.
    lua_pushlstring(L, (const char*)buf, num_bytes);

    return 1;
}
