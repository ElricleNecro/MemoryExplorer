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

// int do_child(int argc, const char **argv)
int do_child(CList l_args)
{
	int nb_args = 0, i = 0;
	for(CList act = l_args; act; act = act->next)
		nb_args++;

	// We are preparing the arguments:
	char *args[nb_args + 1];
	// memcpy(args, argv, nb_args*sizeof(char*));
	for(CList act = l_args; act; act=act->next)
	{
		args[i] = act->opt;
		i++;
	}
	args[nb_args] = NULL;		// The array must be NULL terminated.

#ifdef USE_PTRACE
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);		// We are nofying the system we want to be ptraced!
#endif

	return execvp(args[0], args);	// We launch the command, which will replace the child process.
/*
	char *args[argc + 1];
	memcpy(args, argv, argc*sizeof(char*));
	args[argc] = NULL;		// The array must be NULL terminated.

#ifdef USE_PTRACE
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);		// We are nofying the system we want to be ptraced!
#endif

	return execvp(args[0], args);	// We launch the command, which will replace the child process.
*/
}

int main(int argc, const char **argv)
{
	char *conf = NULL;
	pid_t pid = 0;
	Args *args = Args_New();
	Args_Error err;

	Args_Add(args, "-p", NULL, T_INT, &pid, "Pid to read from.");
	Args_Add(args, "-c", "--conf", T_CHAR, &conf, "LUA configuration file.");

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

	bool error = false;
	bool quit = false;
#ifdef USE_readline
	RLData cli;
	/* char *cmd; */
#endif
	Logger *log = Logger_New(stderr, argv[0], INFO);
	lua_State *L = NULL;
	char input[1024];			//<- the input string...

	{
		Event *ev = NULL;
		if( pid == 0 )
		{
			int nb_args = 0, i = 0;
			for(CList act = args->rest; act; act = act->next)
				nb_args++;

			// We are preparing the arguments:
			const char **l_args = malloc(nb_args * sizeof(char*));
			for(CList act = args->rest; act; act=act->next)
			{
				l_args[i] = act->opt;
				i++;
			}
			ev = Event_NewFromCmd(nb_args, l_args);
			/* Event_Launch(ev); */
			free(l_args);
		}
		else
		{
			ev = Event_New(pid);
		}
		L = MyLua_Init(conf, ev, &quit);
		Logger_Info(log, "Writing pid %d\n", ev->pid);
		free(ev);
	}

	/* event_create_from_c(L, ev); */

	memset(input, 0, 1024 * sizeof(char));	//<- ... is set to 0 everywhere

#ifdef USE_readline
	Logger_Info(
		log,
		"Loading readline.\n"
	);

	RLData_init(&cli, "scanmem > ", "/home/plum/.scanmem_history");
	RLData_readHistory(&cli);
#endif

	Logger_Info(
		log,
		"Beginning loop.\n"
	);

	Args_Free(args);

	while( !quit )			//<- the event loop, where we will interprete all command
	{
#ifdef USE_readline
		RLData_get(&cli);		//<- we call readline to get us the command...

		/* cmd = trim(cli.line);		//<- ... we trimmed it... */

		Logger_Debug(log, "We've get: '%s'\n", ( cli.line )?cli.line:"^D");

		if( !cli.line )
		{
			quit = true;
			continue;
		}

		error = luaL_loadbuffer(L, cli.line, strlen(cli.line), "line") || lua_pcall(L, 0, 0, 0);
		if (error) {
			Logger_Error(
				log,
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
			Logger_Error(
				log,
				"LUA error: '%s'.\n",
				lua_tostring(L, -1)
			);
			lua_pop(L, 1);  /* pop error message from the stack */
		}
#endif
	}

#ifdef USE_readline
	Logger_Info(log, "Saving history to file.\n");
	RLData_saveHistory(&cli);

	RLData_free(&cli);
#endif
	Logger_Free(log);
	MyLua_Free(L);

	return EXIT_SUCCESS;
}

