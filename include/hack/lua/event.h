#ifndef EVENT_H_JBFY4OP2
#define EVENT_H_JBFY4OP2

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

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
void event_create_from_c(lua_State *L, Event *ev);

#endif /* end of include guard: EVENT_H_JBFY4OP2 */

