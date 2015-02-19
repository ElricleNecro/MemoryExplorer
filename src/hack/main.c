#include <ctype.h>
#include <errno.h>
#include <string.h>

#include "dict/dict.h"
#include "hack/hack.h"
#include "logger/logger.h"

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

int main(int argc, char *argv[])
{
	if( argc <= 1 )
	{
		fprintf(stderr, "Call must be: %s (PID)\n", argv[0]);
		return EXIT_FAILURE;
	}

#ifdef USE_readline
	RLData cli;
	char *cmd;
#endif
	char input[1024];			//<- the input string...
	snprintf(input, 1023, "/proc/%s/mem", argv[1]);

	Event ev = {
		.pid = atoi(argv[1]),		//<- pid of the process to read
		.mem_fd = open(
			input,
			O_RDWR
		),				//<- pid memory file
		.log = Logger_new(stderr, ALL),
		.mem = NULL,			//<- memory map
		.end = false,			//<- we do not want to terminate the program right now
		.cmd = NULL,
		.quit = quit,
		.scan = scan,
		.print_map = print_map,
		.print_cmd_dict = print_cmd_dict,
	};					//<- an event structure: will contain each callback action

	Logger_info(
		ev.log,
		"Creating dictionnary.\n"
	);

	;

	if( (ev.cmd = Dict_new(DEF_DICT_SIZE)) == NULL )
	{
		Logger_error(
			ev.log,
			"Unable to create dictionnary: %s\n",
			strerror(errno)
		);
	}

	if( !Dict_set(ev.cmd, "quit", (void*)quit) )
	{
		Logger_error(
			ev.log,
			"Unable to create key '%s': %s\n",
			"quit",
			strerror(errno)
		);
	}
	if( !Dict_set(ev.cmd, "scan", (void*)scan) )
	{
		Logger_error(
			ev.log,
			"Unable to create key '%s': %s\n",
			"scan",
			strerror(errno)
		);
	}
	if( !Dict_set(ev.cmd, "print_map", (void*)print_map) )
	{
		Logger_error(
			ev.log,
			"Unable to create key '%s': %s\n",
			"print_map",
			strerror(errno)
		);
	}
	if( !Dict_set(ev.cmd, "print_cmd_dict", (void*)print_cmd_dict) )
	{
		Logger_error(
			ev.log,
			"Unable to create key '%s': %s\n",
			"print_cmd_dict",
			strerror(errno)
		);
	}

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

	while( !ev.end )			//<- the event loop, where we will interprete all command
	{
#ifdef USE_readline
		RLData_get(&cli);		//<- we call readline to get us the command...

		cmd = trim(cli.line);		//<- ... we trimmed it...

		Logger_debug(ev.log, "We've get: '%s'\n", ( cli.line )?cli.line:"^D");

		/* if( *cmd )			//<- (we are ignoring empty string) */
		interpreting(
			&ev,
			cmd
		);				//<- ... and we interprete it.
#else
		printf("%s > ", argv[0]);
		fgets(
			input,
			1023,
			stdin
		);				//<- we read stdin for commands...
		if(input[strlen(input) - 1] == '\n')
			input[strlen(input) - 1] = '\0';
		interpreting(
			&ev,
			input
		);//<- ... and interprete them.
#endif
	}

#ifdef USE_readline
	Logger_info(ev.log, "Saving history to file.\n");
	RLData_saveHistory(&cli);

	RLData_free(&cli);
#endif
	Dict_free(ev.cmd);
	Maps_free(&ev.mem);
	Logger_free(ev.log);

	return EXIT_SUCCESS;
}

