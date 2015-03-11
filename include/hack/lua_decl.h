#ifndef LUA_DECL_H_6XQI2IYP
#define LUA_DECL_H_6XQI2IYP

#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lautoc.h>

#include "hack/hack.h"

lua_State* lua_Init(void);
void lua_Event(lua_State *L);

#endif /* end of include guard: LUA_DECL_H_6XQI2IYP */

