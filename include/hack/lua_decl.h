#ifndef LUA_H_VEQHA9EL
#define LUA_H_VEQHA9EL

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
// #ifndef luaL_checklong
// #define luaL_checklong(L, n) ((long)luaL_checkinteger(L, (n)))
// #endif

int luaopen_event(lua_State *L);
int luaopen_bool(lua_State *L);

void event_create_from_c(lua_State *L, Event *ev);
void bool_create_from_c(lua_State *L, bool *ev);

lua_State* MyLua_Init(const char *conf, Event *ev, bool *quit);
void MyLua_Free(lua_State *L);


// lua_State* MyLua_Init(Event *ev, const char *conf);
// void MyLua_Event(lua_State *L);
// void MyLua_Free(lua_State *L);

#endif /* end of include guard: LUA_H_VEQHA9EL */

