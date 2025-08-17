#include "lua/lua_bindings.h"
#include "log/log.h"


int l_print(lua_State *L) {
    const char* str = lua_tostring(L, 1);
    LOG_message(LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL, "lua(print): %s", str);
    return 0;
    
}