#include "hack/hack.h"

#ifdef USE_PTRACE
// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
// If the read is succesful, it will print the result
bool scan(Event *ev, char *in)
{
	size_t offset = 0;
	ssize_t bytes_to_read = 4;

	if( sscanf(in, "scan %zd %zd", &offset, &bytes_to_read) < 1 )
	{
		ERROR("incorrect number of argument.");
		return false;
	}
	fprintf(stderr, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	char buf[bytes_to_read];

	ptrace(
		PTRACE_ATTACH,
		ev->pid,
		NULL,
		NULL
	);

	waitpid(ev->pid, NULL, 0);
	/* lseek(ev->mem_fd, offset, SEEK_SET); */
	/* read(ev->mem_fd, buf, bytes_to_read); */
	pread(ev->mem_fd, buf, bytes_to_read, offset);

	printf("%d\n", *((int*)buf));

	ptrace(
		PTRACE_DETACH,
		ev->pid,
		NULL,
		NULL
	);

	/* ptrace(PTRACE_POKEDATA, ev->pid, ) */

	return true;
}
#elif defined(USE_vm_readv)
bool scan(Event *ev, char *in)
{
	size_t offset = 0;
	ssize_t bytes_to_read = 4, nread;

	if( sscanf(in, "scan %zd %zd", &offset, &bytes_to_read) < 1 )
	{
		ERROR("incorrect number of argument.");
		return false;
	}
	fprintf(stderr, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	char buf[bytes_to_read];

	struct iovec local, remote;


	local.iov_base = buf;
	local.iov_len  = bytes_to_read;

	remote.iov_base = (void*)offset;
	remote.iov_len  = bytes_to_read;

	nread = process_vm_readv(
			ev->pid,
			&local,
			1,
			&remote,
			1,
			0
	);

	if( nread != bytes_to_read )
	{
		if( errno == 1 )
			ERROR("operation not permitted");
		else if( errno == 14 )
			ERROR("BAD ADDRESS");
		else
			ERROR("read %zd instead of %zu. Error code %d", nread, bytes_to_read, errno);
		return false;
	}

	return true;
}
#endif

// This function allow the user to quit the program
bool quit(Event *ev, char *in)
{
	(void)in;
	ev->end = true;
	return true;
}

// The interpreter loop, take care of her
bool interpreting(Event *ev, char *input)
{
	if(!strncmp("quit", input, 4))
		return ev->quit(ev, input);
	else if(!strncmp("scan", input, 4))
		return ev->scan(ev, input);

	ERROR("command (%s) not found.", input);

	return false;
}

int main(int argc, char *argv[])
{
	if( argc <= 1 )
	{
		fprintf(stderr, "Call must be: %s (PID)\n", argv[0]);
		return EXIT_FAILURE;
	}

	RLData cli;
	char input[1024];			//<- the input string...
	snprintf(input, 1023, "/proc/%s/mem", argv[1]);

	Event ev = {
		.pid = atoi(argv[1]),		//<- pid of the process to read
		.mem_fd = open(
			input,
			O_RDWR
		),				//<- pid memory file
		.mem = NULL,			//<- memory map
		.end = false,			//<- we do not want to terminate the program right now
		.quit = quit,
		.scan = scan,
	};					//<- an event structure: will contain each callback action

	RLData_init(&cli, "scanmem > ", "~/.scanmem_history");
	RLData_readHistory(&cli);

	Maps_read(&ev.mem, ev.pid);

	memset(input, 0, 1024 * sizeof(char));	//<- ... is set to 0 everywhere

	printf("Writing pid %d\n", ev.pid);

	while( !ev.end )			//<- the event loop, where we will interprete all command
	{
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
	}

	RLData_saveHistory(&cli);

	RLData_free(&cli);
	Maps_free(&ev.mem);

	return EXIT_SUCCESS;
}

