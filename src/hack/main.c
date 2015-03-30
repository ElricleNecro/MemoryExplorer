#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "Parser.h"

#include "hack/hack.h"
#include "logger/logger.h"
#include "hack/lua_decl.h"

#ifdef USE_readline
#include "hack/readline.h"
#endif

static Event ev;

// Taken from: http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *trim(char *str)
{
	if( str == NULL )
		return NULL;

	char *end;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

Event* get_event_instance_ptr(lua_State *L)
{
	return &ev;
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

int do_child(int argc, const char **argv)
{
	// We are preparing the arguments:
	char *args[argc + 1];
	memcpy(args, argv, argc*sizeof(char*));
	args[argc] = NULL;		// The array must be NULL terminated.

#ifdef USE_PTRACE
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);		// We are nofying the system we want to be ptraced!

	/* kill(getpid(), SIGSTOP);	// We are sending ourself a SIGSTOP to allow the parent process to continue. */
#endif

	return execvp(args[0], args);	// We launch the command, which will replace the child process.
}

int main(int argc, const char **argv)
{
	pid_t pid = 0;
	Args *args = Args_New();
	Args_Error err;

	Args_Add(args, "-p", NULL, T_INT, &pid, "Pid to read from.");

	err = Args_Parse(args, argc, argv);
	if( err == TREAT_ERROR )
	{
		Args_Free(args);
		return EXIT_FAILURE;
	}
	else if( err == HELP )
	{
		Args_Free(args);
		return EXIT_SUCCESS;
	}

	if( args->rest && pid == 0)
	{
		pid = fork();
		if( pid == 0 )
			return do_child(argc - 1, argv + 1);
	}
	Args_Free(args);

#ifdef USE_PTRACE
	ptrace(PTRACE_CONT, pid, NULL, NULL);
#endif

	bool error = false;
#ifdef USE_readline
	RLData cli;
	/* char *cmd; */
#endif
	char input[1024];			//<- the input string...
	snprintf(input, 1023, "/proc/%d/mem", pid);

	lua_State *L = lua_Init();

	ev.pid = pid;		//<- pid of the process to read
	ev.mem_fd = open(
		input,
		O_RDWR
	);				//<- pid memory file
	ev.log = Logger_new(stderr, ALL);
	ev.mem = NULL;			//<- memory map
	ev.quit = false;		//<- we do not want to terminate the program right now
	ev._addr = (unsigned long)&ev;

	lua_register(L, "Event_index", Event_index);
	lua_register(L, "Event_newindex", Event_newindex);

	luaL_dofile(L, "init.lua");

#ifdef USE_readline
	Logger_info(
		ev.log,
		"Loading readline.\n"
	);

	RLData_init(&cli, "scanmem > ", "/home/plum/.scanmem_history");
	RLData_readHistory(&cli);
#endif

	Logger_info(
		ev.log,
		"Preparing memory mapping.\n"
	);

	Maps_read(&ev.mem, ev.pid);

	memset(input, 0, 1024 * sizeof(char));	//<- ... is set to 0 everywhere

	Logger_info(ev.log, "Writing pid %d\n", ev.pid);

	Logger_info(
		ev.log,
		"Beginning loop.\n"
	);

	while( !ev.quit )			//<- the event loop, where we will interprete all command
	{
#ifdef USE_readline
		RLData_get(&cli);		//<- we call readline to get us the command...

		/* cmd = trim(cli.line);		//<- ... we trimmed it... */

		Logger_debug(ev.log, "We've get: '%s'\n", ( cli.line )?cli.line:"^D");

		if( !cli.line )
		{
			ev.quit = true;
			continue;
		}

		error = luaL_loadbuffer(L, cli.line, strlen(cli.line), "line") || lua_pcall(L, 0, 0, 0);
		if (error) {
			Logger_error(
				ev.log,
				"LUA error: '%s'.\n",
				lua_tostring(L, -1)
			);
			lua_pop(L, 1);  /* pop error message from the stack */
		}
#else
#warning "Building without readline."
		printf("%s (norl) > ", argv[0]);
		fgets(
			input,
			1023,
			stdin
		);				//<- we read stdin for commands...
		if(input[strlen(input) - 1] == '\n')
			input[strlen(input) - 1] = '\0';
		error = luaL_loadbuffer(L, input, strlen(input), "line") || lua_pcall(L, 0, 0, 0);
		if (error) {
			Logger_error(
				ev.log,
				"LUA error: '%s'.\n",
				lua_tostring(L, -1)
			);
			lua_pop(L, 1);  /* pop error message from the stack */
		}
#endif
	}

#ifdef USE_readline
	Logger_info(ev.log, "Saving history to file.\n");
	RLData_saveHistory(&cli);

	RLData_free(&cli);
#endif
	Maps_free(&ev.mem);
	Logger_free(ev.log);
	lua_close(L);

	return EXIT_SUCCESS;
}

