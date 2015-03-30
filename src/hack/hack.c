#include "hack/hack.h"

#ifdef USE_PTRACE
// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
// If the read is succesful, it will print the result
bool scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out)
{
	/* char nb_str[128] = {0}; */
	/* long offset = 0; */
	/* ssize_t bytes_to_read = 4; */

	/* if( sscanf(in, "scan %s %zd", nb_str, &bytes_to_read) < 1 ) */
	/* { */
		/* Logger_error(ev->log, "incorrect number of argument."); */
		/* return false; */
	/* } */
	/* offset = strtol(nb_str, NULL, 0); */
	Logger_info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	/* char buf[bytes_to_read]; */
	char *buf = NULL;
	if( (buf = malloc(bytes_to_read * sizeof(char))) == NULL )
	{
		Logger_error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

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
	fprintf(stderr, "ptrace %ld\n",
		ptrace(
			PTRACE_PEEKDATA,
			ev->pid,
			offset,
			NULL
		)
	);
	fprintf(stderr, "ptrace %g\n",
		ptrace(
			PTRACE_PEEKDATA,
			ev->pid,
			offset,
			NULL
		)
	);
	fprintf(stderr, "Complement avec ptrace %ld\n",
		~ptrace(
			PTRACE_PEEKDATA,
			ev->pid,
			offset,
			NULL
		) + 1
	);

	Logger_info(ev->log, "Value as int (2complements): '%d'\n", cad( *((int*)( (void*)buf )) ));
	Logger_info(ev->log, "Value as int (bswap64): '%d'\n", bswap_64( *((int*)( (void*)buf )) ));
	Logger_info(ev->log, "Value as int (~bswap64+1): '%d'\n", cad( bswap_64( *((int*)( (void*)buf )) ) ));
	Logger_info(ev->log, "Value as int (~bswap64+1): '%d'\n", bswap_64( cad( *((int*)( (void*)buf )) ) ));

	ptrace(
		PTRACE_DETACH,
		ev->pid,
		NULL,
		NULL
	);

	*(char**)out = buf;

	/* ptrace(PTRACE_POKEDATA, ev->pid, ) */

	return true;
}
#elif defined(USE_vm_readv)
bool scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out)
{
	ssize_t nread;

	Logger_info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	char *buf = NULL;

	struct iovec local, remote;

	if( (buf = malloc(bytes_to_read)) == NULL )
	{
		Logger_error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

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
		Logger_error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

	Logger_info(ev->log, "Value as int (2complements): '%d'\n", cad( *((int*)( (void*)buf )) ));
	Logger_info(ev->log, "Value as int (bswap64): '%d'\n", bswap_64( *((int*)( (void*)buf )) ));
	Logger_info(ev->log, "Value as int (~bswap64+1): '%d'\n", cad( bswap_64( *((int*)( (void*)buf )) ) ));
	Logger_info(ev->log, "Value as int (~bswap64+1): '%d'\n", bswap_64( cad( *((int*)( (void*)buf )) ) ));

	*(char**)out = buf;

	return true;
}
#endif

// This function allow the user to quit the program
bool quit(Event *ev)
{
	ev->quit = true;
	return true;
}

bool print_map(Event *ev)
{
	Logger_debug(
		ev->log,
		"Entering '%s' function\n",
		__func__
	);

	for(Maps *zone=ev->mem; zone != NULL; zone=zone->next)
	{
		if( zone->type == STACK )
			printf("Stack from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == HEAP )
			printf("Heap from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == EXE )
			printf("Exe from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == CODE )
			printf("Code from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		if( zone->read == 'r' )
			printf("\t-> Read permission\n");
		if( zone->write == 'w' )
			printf("\t-> Write permission\n");
		if( zone->exec == 'x' )
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

