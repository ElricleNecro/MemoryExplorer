#include "hack/lua_decl.h"

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

/**
 * Argument check.
 */
Event* event_check(lua_State *L) {
	/* void *user_data = luaL_checkudata(L, 1, "Event"); */
	void *user_data = luaL_testudata(L, 1, "Hack.Event");
	luaL_argcheck(L, user_data != NULL, 1, "'Event' excepted!");
	return (Event*) user_data;
}

/**
 * Creation of the new event struct, from LUA.
 */
int event_new(lua_State *L) {
	Event *ev = (Event *)lua_newuserdata(L, sizeof(struct _event));
	      /* *tmp = Event_New(); */

	luaL_getmetatable(L, "Hack.Event");
	lua_setmetatable(L, -2);

	/* memcpy(ev, tmp, sizeof(struct _event)); */

	return 1;
	(void)ev;
}

/**
 * Create an event struct from C, and making her available from LUA.
 */
void event_create_from_c(lua_State *L, Event *ev) {
	Event *new = (Event *)lua_newuserdata(L, sizeof(struct _event));
	luaL_getmetatable(L, "Hack.Event");
	lua_setmetatable(L, -2);

	memcpy(new, ev, sizeof(struct _event));

	lua_setglobal(L, "ev");
}

/**
 * Method to get quit.
 */
int event_quit(lua_State *L) {
	Event *ev = event_check(L);
	return 1;
	(void)ev;
}

/**
 * Method to get the pid.
 */
int event_pid(lua_State *L) {
	Event *ev = event_check(L);
	lua_pushinteger(L, (lua_Integer)ev->pid);
	return 1;
}

/**
 * Making the event struct printable from LUA.
 */
int event_2string(lua_State *L) {
	Event *ev = event_check(L);
	lua_pushfstring(L, "Process(%d)", ev->pid);
	return 1;
}

/**
 * Freeing the event struct.
 */
int event_free(lua_State *L) {
	Event *ev = event_check(L);

	for(int i=0; ev->args[i] != NULL; i++)
	{
		free(ev->args[i]);
	}
	free(ev->args);

	Maps_free(&ev->mem);
	Logger_Free(ev->log);

	return 0;
}

int event_scan(lua_State *L)
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

int event_write(lua_State *L)
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

int event_run(lua_State *L)
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
	/* {"new", event_new}, */
	{"__tostring", event_2string},
	{"__gc", event_free},
	{"quit", event_quit},
	{"pid", event_pid},
	{"scan", event_scan},
	{"write", event_write},
	{"run", event_run},
	{NULL, NULL},
};

/**
 * Opening our lua binding like a library.
 */
int luaopen_event(lua_State *L) {
	luaL_newmetatable(L, "Hack.Event");

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	luaL_setfuncs(L, Event_m, 0);

	luaL_newlib(L, Event_f);

	return 1;
}

lua_State* MyLua_Init(const char *conf, Event *ev, bool *quit)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_event(L);
	luaopen_bool(L);

	bool_create_from_c(L, quit);
	event_create_from_c(L, ev);

	if( conf )
		luaL_dofile(L, conf);

	return L;
}

void MyLua_Free(lua_State *L)
{
	lua_close(L);
}

