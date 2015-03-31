#include "hack/hack.h"

#if defined(__APPLE__) && defined(__MACH__)
static inline unsigned short bswap_16(unsigned short x) {
	return (x>>8) | (x<<8);
}

static inline unsigned int bswap_32(unsigned int x) {
	return (bswap_16(x&0xffff)<<16) | (bswap_16(x>>16));
}

static inline unsigned long long bswap_64(unsigned long long x) {
	return (((unsigned long long)bswap_32(x&0xffffffffull))<<32) |
		(bswap_32(x>>32));
}
#endif

#ifdef USE_PTRACE
// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
// If the read is succesful, it will print the result
bool scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out)
{
	Logger_info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

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

#ifdef USE_PURE_PTRACE
	char *ptr = buf;

	for(int i = 0; i < bytes_to_read; i+=sizeof(long),ptr+=sizeof(long))
	{
		*ptr = ptrace(
			PTRACE_PEEKDATA,
			ev->pid,
			offset,
			NULL
		);

		printf("%d -- before: %d (read %d -- %d)\n", i, bytes_to_read, (int)*ptr, *ptr);
	}
#elif defined(USE_lseek_read)
	lseek(ev->mem_fd, offset, SEEK_SET);
	read(ev->mem_fd, buf, bytes_to_read);
#else
	pread(ev->mem_fd, buf, bytes_to_read, offset);
#endif

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

	/* Logger_info(ev->log, "Value as int (2complements): '%d'\n", cad( *((int*)( (void*)buf )) )); */
	/* Logger_info(ev->log, "Value as int (bswap64): '%d'\n", bswap_64( *((int*)( (void*)buf )) )); */
	/* Logger_info(ev->log, "Value as int (~bswap64+1): '%d'\n", cad( bswap_64( *((int*)( (void*)buf )) ) )); */
	/* Logger_info(ev->log, "Value as int (~bswap64+1): '%d'\n", bswap_64( cad( *((int*)( (void*)buf )) ) )); */

	*(char**)out = buf;

	return true;
}
#endif

#ifdef USE_PTRACE
// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
// If the read is succesful, it will print the result
bool Event_write(Event *ev, size_t offset, ssize_t bytes_to_write, void *in)
{
	Logger_info(ev->log, "writeing %zu bytes from 0x%zx for pid(%d)\n", bytes_to_write, offset, ev->pid);

	ptrace(
		PTRACE_ATTACH,
		ev->pid,
		NULL,
		NULL
	);

	waitpid(ev->pid, NULL, 0);

#ifdef USE_PURE_PTRACE
	char *ptr = in;

	for(int i = 0; i < bytes_to_write; i+=sizeof(long),ptr+=sizeof(long))
	{
		*ptr = ptrace(
			PTRACE_POKEDATA,
			ev->pid,
			offset,
			ptr
		);

		printf("%d -- before: %d (write %d -- %d)\n", i, bytes_to_write, (int)*ptr, *ptr);
	}
#elif defined(USE_lseek_write)
	lseek(ev->mem_fd, offset, SEEK_SET);
	write(ev->mem_fd, in, bytes_to_write);
#else
	pwrite(ev->mem_fd, in, bytes_to_write, offset);
#endif

	ptrace(
		PTRACE_DETACH,
		ev->pid,
		NULL,
		NULL
	);

	return true;
}
#elif defined(USE_vm_readv)
bool Event_write(Event *ev, size_t offset, ssize_t bytes_to_write, void *in)
{
	Logger_info(ev->log, "writeing %zu bytes from 0x%zx for pid(%d)\n", bytes_to_write, offset, ev->pid);

	ssize_t nwrite;
	struct iovec local, remote;

	local.iov_base = in;
	local.iov_len  = bytes_to_write;

	remote.iov_base = (void*)offset;
	remote.iov_len  = bytes_to_write;

	nwrite = process_vm_writev(
		ev->pid,
		&local,
		1,
		&remote,
		1,
		0
	);

	if( nwrite != bytes_to_write )
	{
		Logger_error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

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

