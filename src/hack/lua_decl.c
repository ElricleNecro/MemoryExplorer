#include "hack/lua_decl.h"

Event *global_ev;

// Converting a pid_t type into a lua integer:
static int luaA_push_pid_t(lua_State *L, luaA_Type t, const void *c_in)
{
	const pid_t *p = (const pid_t*)c_in;

	lua_pushinteger(L, *p);

	return 1;
	(void)t;
}

// Converting a lua integer into a pid_t type:
static void luaA_to_pid_t(lua_State *L, luaA_Type t, void *c_out, int index)
{
	pid_t *p = (pid_t*)c_out;

	*p = lua_tointeger(L, index);

	(void)t;
}

// Get the pointer to the wanted instance of the event structure:
static void luaA_to_pEvent(lua_State *L, luaA_Type t, void *c_out, int index)
{
	Event **p = (Event**)c_out;

	lua_getfield(L, index, "_addr");
	*p = (Event*)lua_tointeger(L, -1);
	lua_remove(L, -1);
	/* lua_pop(L, 1); */

	(void)t;
}

static int C(lua_State *L)
{
	return luaA_call_name(L, lua_tostring(L, 1));
}

int Mylua_Scan(lua_State *L)
{
	if( lua_gettop(L) != 4 )
	{
		lua_pushstring(L, "Invalid number of argument.");
		lua_error(L);

		return 0;
	}

	Event *ev;
	void *out;
	const char *str;
	unsigned long addr = luaL_checklong(L, 2), to_read = luaL_checklong(L, 3);

	// There may be a bug in this function call or around:
	luaA_to_pEvent(L, luaA_type_find(L, "pEvent"), &ev, 1);

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

	if( !Event_Scan(ev, addr, to_read, &out) )
	{
		lua_pushstring(L, "Function 'scan' exited without finishing it's task");
		lua_error(L);
		return 0;
	}

	str = luaL_checkstring(L, 4);

	if( !strcmp(str, "int") )
	{
		Logger_debug(
			ev->log,
			"Representation as int: '%d'\n",
			(int)*(char*)out
		);
		lua_pushinteger(L, (int)*(char*)out);
	}
	else if( !strcmp(str, "long") )
	{
		Logger_debug(
			ev->log,
			"Representation as long: '%ld'\n",
			(long)*(char*)out
		);
		lua_pushinteger(L, (long)*(char*)out);
	}
	else if( !strcmp(str, "float") )
	{
		Logger_debug(
			ev->log,
			"Representation as float: '%g'\n",
			*(float*)out
		);
		lua_pushnumber(L, (float)*(char*)out);
	}
	else if( !strcmp(str, "double") )
	{
		Logger_debug(
			ev->log,
			"Representation as double: '%g'\n",
			*(double*)out
		);
		lua_pushnumber(L, (double)*(char*)out);
	}

	free(out);

	return 1;
}

int Mylua_Write(lua_State *L)
{
	if( lua_gettop(L) != 5 )
	{
		lua_pushstring(L, "Invalid number of argument.");
		lua_error(L);

		return 0;
	}

	Event *ev;
	void *in;
	const char *str;
	unsigned long addr = luaL_checklong(L, 2), to_write = luaL_checklong(L, 3);

	// There may be a bug in this function call or around:
	luaA_to_pEvent(L, luaA_type_find(L, "pEvent"), &ev, 1);

	Logger_debug(
		ev->log,
		"Address to write: '0x%lx'\n",
		addr
	);
	Logger_debug(
		ev->log,
		"Bytes to write: '%ld'\n",
		to_write
	);

	str = luaL_checkstring(L, 4);

	if( !strcmp(str, "int") )
	{
		if( (in = malloc(sizeof(int))) == NULL )
		{
			Logger_error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		Logger_debug(ev->log, "'%s' -- > '%d'\n", lua_tostring(L, 5), luaL_checkint(L, 5));
		*((int*)in) = luaL_checkint(L, 5);
		Logger_debug(
			ev->log,
			"Going to write '%d'\n",
			*((int*)in)
		);
	}
	else if( !strcmp(str, "long") )
	{
		if( (in = malloc(sizeof(long))) == NULL )
		{
			Logger_error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		*((long*)in) = luaL_checklong(L, 5);
		Logger_debug(
			ev->log,
			"Going to write '%ld'\n",
			*((long*)in)
		);
	}
	else if( !strcmp(str, "float") )
	{
		if( (in = malloc(sizeof(float))) == NULL )
		{
			Logger_error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		*((float*)in) = luaL_checkfloat(L, 5);
		Logger_debug(
			ev->log,
			"Going to write '%g'\n",
			*((float*)in)
		);
	}
	else if( !strcmp(str, "double") )
	{
		if( (in = malloc(sizeof(double))) == NULL )
		{
			Logger_error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		*((double*)in) = luaL_checkdouble(L, 5);
		Logger_debug(
			ev->log,
			"Going to write '%g'\n",
			*((double*)in)
		);
	}
	else
	{
		lua_pushstring(L, "No valid type were given!");
		lua_error(L);
		return 0;
	}

	if( !Event_Write(ev, addr, to_write, in) )
	{
		lua_pushstring(L, "Function 'write' exited without finishing it's task");
		lua_error(L);
	}

	free(in);

	return 0;
}

Event* get_event_instance_ptr(lua_State *L)
{
	return global_ev;
	(void)L;
}

// Call when accessing a field:
int Event_index(lua_State *L)
{
	const char *mname = lua_tostring(L, -1);
	/* lua_getfield(L, -1, "_id"); */
	/* printf("Field address: %d\n", lua_tointeger(L, -1)); */
	Event *self = get_event_instance_ptr(L);
	return luaA_struct_push_member_name(L, Event, mname, self);
}

// Call when setting a field:
int Event_newindex(lua_State *L)
{
	const char *mname = lua_tostring(L, -2);
	Event *self = get_event_instance_ptr(L);
	luaA_struct_to_member_name(L, Event, mname, self, -1);
	return 0;
}

lua_State* MyLua_Init(Event *ev)
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

	lua_pushcfunction(L, Mylua_Write);
	lua_setglobal(L, "Writer");

	lua_Event(L);

	lua_register(L, "Event_index", Event_index);
	lua_register(L, "Event_newindex", Event_newindex);

	global_ev = ev;

	luaL_dofile(L, "init.lua");

	return L;
}

luaA_function_declare(Event_Quit, bool, Event*)
luaA_function_declare(Event_PrintMap, bool, Event*)
/* luaA_function_declare(scan, bool, Event*, unsigned long, unsigned long) */

void MyLua_Event(lua_State *L)
{
	luaA_struct(L, Event);
	luaA_struct_member(L, Event, pid, pid_t);
	luaA_struct_member(L, Event, quit, bool);
	luaA_struct_member(L, Event, _addr, unsigned long);

	luaA_function_register(L, Event_Quit, bool, Event*);
	luaA_function_register(L, Event_PrintMap, bool, Event*);
	/* luaA_function_register(L, scan, bool, Event*, unsigned long, unsigned long); */
}

void MyLua_Free(lua_State *L)
{
	lua_close(L);
	/* free(ev); */
}
