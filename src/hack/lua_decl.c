#include "hack/lua_decl.h"

static int luaA_push_pid_t(lua_State *L, luaA_Type t, const void *c_in)
{
	pid_t *p = (pid_t*)c_in;

	lua_pushinteger(L, *p);

	return 1;
	(void)t;
}

static void luaA_to_pid_t(lua_State *L, luaA_Type t, void *c_out, int index)
{
	pid_t *p = (pid_t*)c_out;

	*p = lua_tointeger(L, index);

	(void)t;
}

static void luaA_to_pEvent(lua_State *L, luaA_Type t, void *c_out, int index)
{
	Event **p = (Event**)c_out;

	lua_getfield(L, index, "_addr");
	*p = (Event*)lua_tointeger(L, -1);
	lua_pop(L, 1);

	(void)t;
}

static int C(lua_State *L)
{
	return luaA_call_name(L, lua_tostring(L, 1));
}

lua_State* lua_Init(void)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaA_open(L);

	luaA_conversion(L, bool, luaA_push_bool, luaA_to_bool);
	luaA_conversion(L, pid_t, luaA_push_pid_t, luaA_to_pid_t);

	luaA_conversion(L, Event*, NULL, luaA_to_pEvent);

	lua_register(L, "C_call", C);

	lua_Event(L);

	return L;
}

luaA_function_declare(quit, bool, Event*)
luaA_function_declare(print_map, bool, Event*)
luaA_function_declare(scan, bool, Event*, size_t, ssize_t)

void lua_Event(lua_State *L)
{
	luaA_struct(L, Event);
	luaA_struct_member(L, Event, pid, pid_t);
	luaA_struct_member(L, Event, quit, bool);
	luaA_struct_member(L, Event, _addr, unsigned long);

	luaA_function_register(L, quit, bool, Event*);
	luaA_function_register(L, print_map, bool, Event*);
	luaA_function_register(L, scan, bool, Event*, unsigned long, unsigned long);
}

