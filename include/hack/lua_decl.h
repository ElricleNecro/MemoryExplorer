#ifndef LUA_H_VEQHA9EL
#define LUA_H_VEQHA9EL

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "hack/hack.h"
#include "hack/lua/bool.h"
#include "hack/lua/event.h"

lua_State* MyLua_Init(const char *conf, Event *ev, bool *quit);
void MyLua_Free(lua_State *L);

#endif /* end of include guard: LUA_H_VEQHA9EL */

