#ifndef LUA_DECL_H_6XQI2IYP
#define LUA_DECL_H_6XQI2IYP

#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lautoc.h>

#include "hack/hack.h"

#ifndef luaL_checkfloat
#define luaL_checkfloat(L, n) ((float)luaL_checknumber(L, (n)))
#endif
#ifndef luaL_checkdouble
#define luaL_checkdouble(L, n) ((double)luaL_checknumber(L, (n)))
#endif

lua_State* MyLua_Init(Event *ev, const char *conf);
void MyLua_Event(lua_State *L);
void MyLua_Free(lua_State *L);

#endif /* end of include guard: LUA_DECL_H_6XQI2IYP */

