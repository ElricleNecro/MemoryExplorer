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
		Logger_error(ev->log, "incorrect number of argument.");
		return false;
	}
	Logger_info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

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

	Logger_info(ev->log, "%d\n", *((int*)buf));

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
		Logger_error(ev->log, "incorrect number of argument.");
		return false;
	}
	Logger_info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

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
			Logger_error(ev->log, "operation not permitted");
		else if( errno == 14 )
			Logger_error(ev->log, "BAD ADDRESS");
		else
			Logger_error(ev->log, "read %zd instead of %zu. Error code %d", nread, bytes_to_read, errno);
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

bool print_map(Event *ev, char *in)
{
	Logger_debug(
		ev->log,
		"Entering '%s' function\n",
		__func__
	);
	for(Maps *zone=ev->mem; zone != NULL; zone=zone->next)
	{
		if( zone->type == STACK )
			printf("Stack from %lxu -> %lxu (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == HEAP )
			printf("Heap from %lxu -> %lxu (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == EXE )
			printf("Exe from %lxu -> %lxu (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == CODE )
			printf("Code from %lxu -> %lxu (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		if( zone->read == 'r' )
			printf("\t-> Read permission\n");
		if( zone->read == 'w' )
			printf("\t-> Write permission\n");
		if( zone->read == 'x' )
			printf("\t-> Execution permission\n");
		printf("\t-> %s\n", zone->filename);
	}
	Logger_debug(
		ev->log,
		"Leaving '%s' function\n",
		__func__
	);

	return true;
}

// The interpreter loop, take care of her
bool interpreting(Event *ev, char *input)
{
	if(!strncmp("quit", input, 4))
		return ev->quit(ev, input);
	else if(!strncmp("scan", input, 4))
		return ev->scan(ev, input);
	else if(!strncmp("print_map", input, 9))
		return ev->print_map(ev, input);

	Logger_error(ev->log, "command (%s) not found.", input);

	return false;
}

