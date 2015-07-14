#include "hack/lua/event.h"

/**
 * Argument check.
 */
static Event* event_check(lua_State *L) {
	/* void *user_data = luaL_checkudata(L, 1, "Event"); */
	void *user_data = luaL_testudata(L, 1, "Event");
	luaL_argcheck(L, user_data != NULL, 1, "'Event' excepted!");
	return (Event*) user_data;
}

/**
 * Creation of the new event struct, from LUA.
 */
static int event_new(lua_State *L)
{
	Event *ev = (Event *)lua_newuserdata(L, sizeof(struct _event));
	      /* *tmp = Event_New(); */

	luaL_getmetatable(L, "Event");
	lua_setmetatable(L, -2);

	/* memcpy(ev, tmp, sizeof(struct _event)); */
	ev->args = NULL;
	ev->mem = NULL;
	ev->log = NULL;

	return 1;
	(void)ev;
}

/**
 * Create an event struct from C, and making her available from LUA.
 */
void event_create_from_c(lua_State *L, Event *ev)
{
	Event *new = (Event *)lua_newuserdata(L, sizeof(struct _event));
	luaL_getmetatable(L, "Event");
	lua_setmetatable(L, -2);

	memcpy(new, ev, sizeof(struct _event));

	lua_setglobal(L, "ev");
}

/**
 * Method to get the pid.
 */
static int event_pid(lua_State *L)
{
	Event *ev = event_check(L);

	lua_pushinteger(L, (lua_Integer)ev->pid);

	return 1;
}

/**
 * Making the event struct printable from LUA.
 */
static int event_2string(lua_State *L)
{
	Event *ev = event_check(L);

	lua_pushfstring(L, "Process(%d)", ev->pid);

	return 1;
}

/**
 * Freeing the event struct.
 */
static int event_free(lua_State *L)
{
	Event *ev = event_check(L);

	if( ev->args != NULL )
	{
		for(int i=0; ev->args[i] != NULL; i++)
		{
			free(ev->args[i]);
		}
		free(ev->args);
	}

	if( ev->mem != NULL )
		Maps_free(&ev->mem);
	if( ev->log != NULL )
		Logger_Free(ev->log);

	return 0;
}

static int event_scan(lua_State *L)
{
	if( lua_gettop(L) != 4 )
	{
		lua_pushstring(L, "Invalid number of argument.");
		lua_error(L);

		return 0;
	}

	Event *ev = event_check(L);
	void *out;
	const char *str;
	unsigned long addr = luaL_checklong(L, 2), to_read = luaL_checklong(L, 3);

	Logger_Debug(
		ev->log,
		"Address to read: '0x%lx'\n",
		addr
	);
	Logger_Debug(
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
		Logger_Debug(
			ev->log,
			"Representation as int: '%d'\n",
			(int)*(char*)out
		);
		lua_pushinteger(L, (int)*(char*)out);
	}
	else if( !strcmp(str, "long") )
	{
		Logger_Debug(
			ev->log,
			"Representation as long: '%ld'\n",
			(long)*(char*)out
		);
		lua_pushinteger(L, (long)*(char*)out);
	}
	else if( !strcmp(str, "float") )
	{
		Logger_Debug(
			ev->log,
			"Representation as float: '%g'\n",
			*(float*)out
		);
		lua_pushnumber(L, (float)*(char*)out);
	}
	else if( !strcmp(str, "double") )
	{
		Logger_Debug(
			ev->log,
			"Representation as double: '%g'\n",
			*(double*)out
		);
		lua_pushnumber(L, (double)*(char*)out);
	}

	free(out);

	return 1;
}

static int event_write(lua_State *L)
{
	if( lua_gettop(L) != 5 )
	{
		lua_pushstring(L, "Invalid number of argument.");
		lua_error(L);

		return 0;
	}

	Event *ev = event_check(L);
	void *in;
	const char *str;
	unsigned long addr = luaL_checklong(L, 2), to_write = luaL_checklong(L, 3);

	Logger_Debug(
		ev->log,
		"Address to write: '0x%lx'\n",
		addr
	);
	Logger_Debug(
		ev->log,
		"Bytes to write: '%ld'\n",
		to_write
	);

	str = luaL_checkstring(L, 4);

	if( !strcmp(str, "int") )
	{
		if( (in = malloc(sizeof(int))) == NULL )
		{
			Logger_Error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		Logger_Debug(ev->log, "'%s' -- > '%d'\n", lua_tostring(L, 5), luaL_checkint(L, 5));
		*((int*)in) = luaL_checkint(L, 5);
		Logger_Debug(
			ev->log,
			"Going to write '%d'\n",
			*((int*)in)
		);
	}
	else if( !strcmp(str, "long") )
	{
		if( (in = malloc(sizeof(long))) == NULL )
		{
			Logger_Error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		*((long*)in) = luaL_checklong(L, 5);
		Logger_Debug(
			ev->log,
			"Going to write '%ld'\n",
			*((long*)in)
		);
	}
	else if( !strcmp(str, "float") )
	{
		if( (in = malloc(sizeof(float))) == NULL )
		{
			Logger_Error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		*((float*)in) = luaL_checkfloat(L, 5);
		Logger_Debug(
			ev->log,
			"Going to write '%g'\n",
			*((float*)in)
		);
	}
	else if( !strcmp(str, "double") )
	{
		if( (in = malloc(sizeof(double))) == NULL )
		{
			Logger_Error(
				ev->log,
				"Allocation error: '%s'\n",
				strerror(errno)
			);
		}
		*((double*)in) = luaL_checkdouble(L, 5);
		Logger_Debug(
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

static int event_run(lua_State *L)
{
	if( lua_gettop(L) != 1 )
	{
		lua_pushstring(L, "Invalid number of argument.");
		lua_error(L);

		return 0;
	}

	Event *ev = event_check(L);

	Event_Launch(ev);

	return 0;
}

static int event_read_map(lua_State *L)
{
	Event *ev = event_check(L);

	Event_ReadMap(ev);

	return 0;
}

static int event_print_map(lua_State *L)
{
	Event *ev = event_check(L);

	Event_PrintMap(ev);

	return 0;
}

/**
 * Setting event associated function.
 */
static const struct luaL_Reg Event_f[] = {
	{"new", event_new},
	{NULL, NULL},
};

/**
 * Setting event associated method.
 */
static const struct luaL_Reg Event_m[] = {
	{"__tostring", event_2string},
	{"__gc", event_free},
	{"pid", event_pid},
	{"scan", event_scan},
	{"write", event_write},
	{"run", event_run},
	{"print_map", event_print_map},
	{"read_map", event_read_map},
	{NULL, NULL},
};

/**
 * Opening our lua binding like a library.
 */
int luaopen_event(lua_State *L) {
	luaL_newmetatable(L, "Event");

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	luaL_setfuncs(L, Event_m, 0);

	luaL_newlib(L, Event_f);

	return 1;
}

