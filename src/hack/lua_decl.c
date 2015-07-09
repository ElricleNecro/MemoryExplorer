#include "hack/lua_decl.h"

lua_State* MyLua_Init(const char *conf, Event *ev, bool *quit)
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	/* luaopen_event(L); */
	luaopen_bool(L);
	lua_register(L, "open_event", luaopen_event);

	if( conf )
		luaL_dofile(L, conf);

	bool_create_from_c(L, quit);
	event_create_from_c(L, ev);

	return L;
}

void MyLua_Free(lua_State *L)
{
	lua_close(L);
}

