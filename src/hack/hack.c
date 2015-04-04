#include "hack/hack.h"

#if defined(__APPLE__) && defined(__MACH__) // {
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
#endif // }

Event* Event_New(pid_t pid, const char *mem_file)
{
	Event *ev = NULL;

	if( (ev = malloc(sizeof(Event))) == NULL )
	{
		perror("Allocation error:");
		return NULL;
	}

	ev->pid = pid;		//<- pid of the process to read
	ev->mem_fd = open(
		mem_file,
		O_RDWR
	);				//<- pid memory file
	ev->log = Logger_new(stderr, ALL);
	ev->mem = NULL;			//<- memory map
	ev->quit = false;		//<- we do not want to terminate the program right now
	ev->_addr = (unsigned long)ev;

	Logger_info(
		ev->log,
		"Preparing memory mapping.\n"
	);

	Maps_read(&ev->mem, ev->pid);

	return ev;
}

void Event_Free(Event *ev)
{
	Maps_free(&ev->mem);
	Logger_free(ev->log);

	free(ev);
}

#ifdef USE_PTRACE // {
bool Event_Attach(Event *ev)
{
	int status;

	if( ptrace(PTRACE_ATTACH, ev->pid, NULL, NULL) == -1L )
	{
		Logger_error(
			ev->log,
			"Error while attaching the process: '%s'.",
			strerror(errno)
		);
		return false;
	}

	if( ( waitpid(ev->pid, &status, 0) == -1L ) || !WIFSTOPPED(status) )
	{
		Logger_error(
			ev->log,
			"Error while stopping the process: '%s'.",
			strerror(errno)
		);
		return false;
	}

	return true;
}

bool Event_Detach(Event *ev)
{
	if( ptrace(PTRACE_DETACH, ev->pid, NULL, NULL) == -1L)
	{
		Logger_error(
			ev->log,
			"Error while detaching the process: '%s'.",
			strerror(errno)
		);
		return false;
	}

	return true;
}

// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
// If the read is succesful, it will print the result
bool Event_Scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out)
{
	Logger_info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	char *buf = NULL;
	size_t to_allocate = bytes_to_read * sizeof(char);

#ifdef USE_PURE_PTRACE
	to_allocate += to_allocate % sizeof(long);
#endif
	if( (buf = malloc(to_allocate)) == NULL )
	{
		Logger_error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

	if( !Event_Attach(ev) )
		return false;

#ifdef USE_PURE_PTRACE
	char *ptr = buf;

	/* for(int i = 0; i < bytes_to_read; i+=sizeof(long),ptr+=sizeof(long)) */
	/* { */
		*buf = ptrace(
			PTRACE_PEEKDATA,
			ev->pid,
			offset,
			NULL
		);

		/* printf("%d -- before: %d (read %d -- %d)\n", i, bytes_to_read, (int)*ptr, *ptr); */
	/* } */
#elif defined(USE_lseek_read)
	lseek(ev->mem_fd, offset, SEEK_SET);
	read(ev->mem_fd, buf, bytes_to_read);
#else
	pread(ev->mem_fd, buf, bytes_to_read, offset);
#endif

	*(char**)out = buf;

	/* ptrace(PTRACE_POKEDATA, ev->pid, ) */

	return Event_Detach(ev);
}

// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
// If the read is succesful, it will print the result
bool Event_Write(Event *ev, size_t offset, ssize_t bytes_to_write, void *in)
{
	Logger_info(ev->log, "writeing %zu bytes from 0x%zx for pid(%d)\n", bytes_to_write, offset, ev->pid);

#ifdef USE_PURE_PTRACE
	void *data = NULL;

	// If we want to write something smaller than a word (sizeof(long)), we are going to need the surrounding memory.
	if( bytes_to_write < sizeof(long) )
	{
		scan(ev, offset, bytes_to_write, &data);

		memcpy(data, in, bytes_to_write);
		/* for (int i = 0; i < bytes_to_write; ++i) */
		/* { */
			/* ((char*)data)[i] = ((char*)in)[i]; */
		/* } */
	}
	else
		data = in;
#endif

	if( !Event_Attach(ev) )
		return false;

#ifdef USE_PURE_PTRACE

	ptrace(
		PTRACE_POKEDATA,
		ev->pid,
		offset,
		data
	);

#elif defined(USE_lseek_write)
	lseek(ev->mem_fd, offset, SEEK_SET);
	write(ev->mem_fd, in, bytes_to_write);
#else
	pwrite(ev->mem_fd, in, bytes_to_write, offset);
#endif

	return Event_Detach(ev);
}
// }
#elif defined(USE_vm_readv) // {
bool Event_Scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out)
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

	*(char**)out = buf;

	return true;
}

bool Event_Write(Event *ev, size_t offset, ssize_t bytes_to_write, void *in)
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
#endif // }

// This function allow the user to quit the program
bool Event_Quit(Event *ev)
{
	ev->quit = true;
	return true;
}

bool Event_PrintMap(Event *ev)
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

