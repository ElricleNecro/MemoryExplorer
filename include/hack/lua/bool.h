#ifndef BOOL_H_ZQUAFDHN
#define BOOL_H_ZQUAFDHN

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdbool.h>

int luaopen_bool(lua_State *L);
void bool_create_from_c(lua_State *L, bool *ev);

#endif /* end of include guard: BOOL_H_ZQUAFDHN */

