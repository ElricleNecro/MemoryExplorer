#include "logger/logger.h"
#include "hack/hack.h"

#ifdef USE_readline
#include "hack/readline.h"
#endif

int main(int argc, char *argv[])
{
	if( argc <= 1 )
	{
		fprintf(stderr, "Call must be: %s (PID)\n", argv[0]);
		return EXIT_FAILURE;
	}

#ifdef USE_readline
	RLData cli;
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
		.quit = quit,
		.scan = scan,
		.print_map = print_map,
	};					//<- an event structure: will contain each callback action

	Logger_info(
		ev.log,
		"Loading readline.\n"
	);

#ifdef USE_readline
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
		RLData_get(&cli);

		Logger_debug(ev.log, "We've get: '%s'\n", cli.line);

		if( cli.line && *cli.line )
		{
			Logger_debug(ev.log, "Ready to interprete: '%s'\n", cli.line);
			interpreting(
					&ev,
					cli.line
				    );
		}
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
	Maps_free(&ev.mem);
	Logger_free(ev.log);

	return EXIT_SUCCESS;
}

