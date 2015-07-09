#include "hack/lua/bool.h"

/**
 * Argument check.
 */
bool** bool_check(lua_State *L) {
	/* void *user_data = luaL_checkudata(L, 1, "Event"); */
	void *user_data = luaL_testudata(L, 1, "bool");
	luaL_argcheck(L, user_data != NULL, 1, "'Event' excepted!");
	return (bool**) user_data;
}

/**
 * Create an bool struct from C, and making her available from LUA.
 */
void bool_create_from_c(lua_State *L, bool *ev) {
	bool **new = (bool **)lua_newuserdata(L, sizeof(bool*));
	luaL_getmetatable(L, "bool");
	lua_setmetatable(L, -2);

	*new = ev;

	lua_setglobal(L, "quit");
}

/**
 * Function call to quit the program.
 */
int quit_call(lua_State *L) {
	bool **new = bool_check(L);
	**new = true;
	return 0;
}

static const struct luaL_Reg bool_m[] = {
	{"__call", quit_call},
	{NULL, NULL},
};

/**
 * Opening our lua binding like a library.
 */
int luaopen_bool(lua_State *L) {
	luaL_newmetatable(L, "bool");

	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);		// push the table
	lua_pushvalue(L, -3);		// metatable.__index = metatable

	luaL_setfuncs(L, bool_m, 0);

	return 1;
}

