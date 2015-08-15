#include "hack/lua_decl.h"

lua_State* MyLua_Init(const char *conf, Event *ev, bool *quit)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	/* luaopen_event(L); */
	luaopen_bool(L);
	/* lua_register(L, "open_event", luaopen_event); */
	luaopen_event(L);
	lua_setglobal(L, "Event");

	if( conf )
		if( luaL_dofile(L, conf) )
			Logger_Error(ev->log, "Unable do run file '%s': '%s'\n", conf, lua_tostring(L, -1));

	bool_create_from_c(L, quit);
	event_create_from_c(L, ev);

	return L;
}

void MyLua_Free(lua_State *L)
{
	lua_close(L);
}

