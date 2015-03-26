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

int Mylua_Scan(lua_State *L)
{
	Event *ev;
	int size = lua_gettop(L);
	void *out;
	const char *str;
	unsigned long addr, to_read;

	if( size != 4 )
	{
		lua_pushstring(L, "Invalid number of argument.");
		lua_error(L);

		return 0;
	}

	luaA_to_pEvent(L, luaA_type_find(L, "pEvent"), &ev, 1);

	/* if( !lua_isnumber(L, 2) ) */
	/* { */
		/* lua_pushstring(L, "Incorrect argument type"); */
		/* lua_error(L); */

		/* return 0; */
	/* } */
	Logger_debug(
		ev->log,
		"Arg %d: '%s'",
		2,
		lua_tostring(L, 2)
	);
	addr = luaL_checklong(L, 2);

	/* if( !lua_isnumber(L, 3) ) */
	/* { */
		/* lua_pushstring(L, "Incorrect argument type"); */
		/* lua_error(L); */

		/* return 0; */
	/* } */
	Logger_debug(
		ev->log,
		"Arg %d: '%s'",
		3,
		lua_tostring(L, 3)
	);
	to_read = luaL_checklong(L, 3);

	Logger_debug(
		ev->log,
		"Address to read: '0x%lx'\n",
		addr
	);
	Logger_debug(
		ev->log,
		"Bytes to read: '%ld'\n",
		to_read
	);

	if( !scan(ev, addr, to_read, &out) )
	{
		lua_pushstring(L, "Function 'scan' exited without finishing it's task");
		lua_error(L);
		return 0;
	}

	str = luaL_checkstring(L, 4);

	if( strcmp(str, "int") )
		lua_pushinteger(L, *(int*)out);
	else if( strcmp(str, "long") )
		lua_pushinteger(L, *(long*)out);
	else if( strcmp(str, "float") )
		lua_pushnumber(L, *(float*)out);
	else if( strcmp(str, "double") )
		lua_pushnumber(L, *(double*)out);

	free(out);

	return 1;
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

	lua_pushcfunction(L, Mylua_Scan);
	lua_setglobal(L, "Scanner");

	lua_Event(L);

	return L;
}

luaA_function_declare(quit, bool, Event*)
luaA_function_declare(print_map, bool, Event*)
/* luaA_function_declare(scan, bool, Event*, unsigned long, unsigned long) */

void lua_Event(lua_State *L)
{
	luaA_struct(L, Event);
	luaA_struct_member(L, Event, pid, pid_t);
	luaA_struct_member(L, Event, quit, bool);
	luaA_struct_member(L, Event, _addr, unsigned long);

	luaA_function_register(L, quit, bool, Event*);
	luaA_function_register(L, print_map, bool, Event*);
	/* luaA_function_register(L, scan, bool, Event*, unsigned long, unsigned long); */
}

