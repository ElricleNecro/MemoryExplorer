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

Event* Event_New(pid_t pid)
{
	Event *ev = NULL;

	if( (ev = malloc(sizeof(Event))) == NULL )
	{
		perror("Allocation error:");
		return NULL;
	}

	ev->log = Logger_New(stderr, "Process", ALL);
	ev->mem = NULL;			//<- memory map

	Event_SetPID(ev, pid);

	return ev;
}

Event* Event_NewFromCmd(const int nb_args, const char **args)
{
	Event *ev = NULL;

	if( (ev = malloc(sizeof(Event))) == NULL )
	{
		perror("Allocation error:");
		return NULL;
	}

	ev->pid = 0;		//<- pid of the process to read
	ev->log = Logger_New(stderr, "Process", ALL);
	ev->mem = NULL;			//<- memory map

	ev->nb_args = nb_args;
	if( (ev->args = malloc(sizeof(char*) * ( ev->nb_args + 1 ))) == NULL )
	{
		perror("Allocation error:");
		Logger_Free(ev->log);
		free(ev);

		return NULL;
	}

	ev->args[ ev->nb_args ] = NULL;
	for(int i=0; i<nb_args; i++)
	{
		ev->args[i] = malloc(sizeof(char) * strlen(args[i]));
		strcpy(ev->args[i], args[i]);
	}

	return ev;
}

void Event_SetPID(Event *ev, pid_t pid)
{
	char mem_file[1024];

	ev->pid = pid;		//<- pid of the process to read
	snprintf(mem_file, 1023, "/proc/%d/mem", ev->pid);

	ev->mem_fd = open(
		mem_file,
		O_RDWR
	);				//<- pid memory file

	Logger_Info(
		ev->log,
		"Preparing memory mapping.\n"
	);
}

void Event_ReadMap(Event *ev)
{
	Maps_read(&ev->mem, ev->pid);
}

static int do_child(char **cmd)
{
	// Launch the command.
#ifdef USE_PTRACE
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);		// We are nofying the system we want to be ptraced!
#endif
	return execvp(cmd[0], cmd);
}

void Event_Launch(Event *ev)
{
	if( ev->pid == 0 )
	{
		ev->pid = fork();
		if( ev->pid == 0 )
			exit(do_child(ev->args));
#ifdef USE_PTRACE // {
#ifdef DEBUG_SIGNALS // {
		int status;
		waitpid(ev->pid, &status, 0);
		if( WIFCONTINUED(status) )
			fprintf(stderr, "Pid %d continued\n", ev->pid);
		else if( WIFSTOPPED(status) )
			fprintf(stderr, "Pid %d is stopped.\n", ev->pid);
		else
			fprintf(stderr, "Pid %d has status %d.\n", ev->pid, status);
#else
		waitpid(ev->pid, NULL, 0);
#endif // }
		ptrace(PTRACE_DETACH, ev->pid, NULL, NULL);
#endif // }
	}

	Event_SetPID(ev, ev->pid);
}

void Event_Free(Event *ev)
{
	Maps_free(&ev->mem);
	Logger_Free(ev->log);

	free(ev);
}

#ifdef USE_PTRACE // {
bool Event_Attach(Event *ev)
{
	int status;

	if( ptrace(PTRACE_ATTACH, ev->pid, NULL, NULL) == -1L )
	{
		Logger_Error(
			ev->log,
			"Error while attaching the process: '%s'.",
			strerror(errno)
		);
		return false;
	}

	if( ( waitpid(ev->pid, &status, 0) == -1L ) || !WIFSTOPPED(status) )
	{
		Logger_Error(
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
		Logger_Error(
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
	Logger_Info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	int8_t *buf = NULL;
	size_t to_allocate = bytes_to_read * sizeof(int8_t);

#ifdef USE_PURE_PTRACE
	to_allocate += to_allocate % sizeof(long);
#endif
	if( (buf = malloc(to_allocate)) == NULL )
	{
		Logger_Error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

	if( !Event_Attach(ev) )
		return false;

#ifdef USE_PURE_PTRACE
	int8_t *ptr = buf;

	for(int i = 0; i < bytes_to_read; i+=sizeof(long),ptr+=sizeof(long))
	{
		*ptr = ptrace(
			PTRACE_PEEKDATA,
			ev->pid,
			offset + i,
			NULL
		);
		/* *buf = ptrace( */
			/* PTRACE_PEEKDATA, */
			/* ev->pid, */
			/* offset, */
			/* NULL */
		/* ); */

		/* printf("%d -- before: %d (read %d -- %d)\n", i, bytes_to_read, (int)*ptr, *ptr); */
	}
#elif defined(USE_lseek_read)
	lseek(ev->mem_fd, offset, SEEK_SET);
	read(ev->mem_fd, buf, bytes_to_read);
#else
	pread(ev->mem_fd, buf, bytes_to_read, offset);
#endif

	*(int8_t**)out = buf;

	/* ptrace(PTRACE_POKEDATA, ev->pid, ) */

	return Event_Detach(ev);
}

// This function will scan the memory. It takes an argument which is the memory offset to read from the process `ev->pid`.
bool Event_Write(Event *ev, size_t offset, ssize_t bytes_to_write, void *in)
{
	Logger_Info(ev->log, "writing %zu bytes from 0x%zx for pid(%d)\n", bytes_to_write, offset, ev->pid);

#ifdef USE_PURE_PTRACE
	void *data = NULL;
	bool deallocate = false;

	// Number of bytes to write:
	size_t to_write = bytes_to_write * sizeof(int8_t);
	// in case we are writing something smaller/bigger than a word:
	to_write += to_write % sizeof(long);

	Logger_Debug(ev->log, "We are going to write %zu bytes and so need to read first %zu bytes.\n", bytes_to_write, to_write);
	Logger_Debug(ev->log, "%zu %% %zu == %zu\n", to_write, sizeof(long), to_write % sizeof(long));

	// If we want to write something smaller than a word (sizeof(long)), we are going to need the surrounding memory.
	if( ( bytes_to_write * sizeof(int8_t) ) % sizeof(long) != 0 )
	{
		// As Event_Scan allocate memory, we need to free it at the end:
		deallocate = true;
		Logger_Debug(ev->log, "Reading the %zu bytes...\n", to_write);
		// Reading the chunk we need:
		Event_Scan(ev, offset, bytes_to_write, &data);

		// For debug purpose we check the actual value:
		Logger_Debug(ev->log, "Researched value: %d\n", (int)*(int8_t*)data);

		// Writing our data into the chunk:
		memcpy(data, in, bytes_to_write * sizeof(int8_t));

		// For debug purpose we check the new value:
		Logger_Debug(ev->log, "Wanted value: %d\n", (int)*(int8_t*)data);
	}
	else
		// If our data as a length correponding to a word:
		data = in;
#endif

	// Attaching the process to be able to write:
	if( !Event_Attach(ev) )
		return false;

#ifdef USE_PURE_PTRACE
	void *ptr = data;

	// In case we have to write multiple word, we are looping:
	for(size_t i=0; i<to_write; i+=sizeof(long),ptr+=sizeof(long))
	{
		// Checking the step of the loop:
		Logger_Debug(ev->log, "i = %zu / %zu\n", i, to_write);

		// Writing a word into the process memory:
		ptrace(
			PTRACE_POKEDATA,
			ev->pid,
			offset,
			/* &val */
			ptr
		);
	}

	// Freeing the Event_Scan memory if needed:
	if( deallocate )
		free(data);

#elif defined(USE_lseek_write)
	lseek(ev->mem_fd, offset, SEEK_SET);
	write(ev->mem_fd, in, bytes_to_write);
#else
	pwrite(ev->mem_fd, in, bytes_to_write, offset);
#endif

	// Detaching it:
	return Event_Detach(ev);
}
// }
#elif defined(USE_vm_readv) // {
bool Event_Scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out)
{
	ssize_t nread;

	Logger_Info(ev->log, "Reading %zu bytes from 0x%zx for pid(%d)\n", bytes_to_read, offset, ev->pid);

	char *buf = NULL;

	struct iovec local, remote;

	if( (buf = malloc(bytes_to_read)) == NULL )
	{
		Logger_Error(
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

	errno = 0;
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
		Logger_Error(
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
	Logger_Info(ev->log, "writeing %zu bytes from 0x%zx for pid(%d)\n", bytes_to_write, offset, ev->pid);

	ssize_t nwrite;
	struct iovec local, remote;

	local.iov_base = in;
	local.iov_len  = bytes_to_write;

	remote.iov_base = (void*)offset;
	remote.iov_len  = bytes_to_write;

	errno = 0;
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
		Logger_Error(
			ev->log,
			"Allocation error: '%s'\n",
			strerror(errno)
		);
		return false;
	}

	return true;
}
#endif // }

bool Event_PrintMap(Event *ev)
{
	Logger_Debug(
		ev->log,
		"Entering '%s' function\n",
		__func__
	);

	for(Maps zone=ev->mem; zone != NULL; zone=zone->next)
	{
		if( zone->type == STACK )
			printf("Stack from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == HEAP )
			printf("Heap from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == EXE )
			printf("Exe from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else if( zone->type == CODE )
			printf("Code from 0x%lx -> 0x%lx (%lu)\n", zone->start, zone->end, zone->end - zone->start);
		else
			printf("Something else...");
		if( zone->read == 'r' )
			printf("\t-> Read permission\n");
		if( zone->write == 'w' )
			printf("\t-> Write permission\n");
		if( zone->exec == 'x' )
			printf("\t-> Execution permission\n");
		printf("\t-> %s\n", zone->filename);
	}

	Logger_Debug(
		ev->log,
		"Leaving '%s' function\n",
		__func__
	);

	return true;
}

