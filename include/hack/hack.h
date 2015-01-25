#ifndef HACK_H_326FWU4H
#define HACK_H_326FWU4H

#if defined(USE_PTRACE) && defined(USE_vm_readv)
	#undef USE_vm_readv
#elif !defined(USE_PTRACE) && !defined(USE_vm_readv)
	#define USE_PTRACE
#endif

#ifdef USE_PTRACE
	#include <sys/ptrace.h>
	#include <sys/wait.h>
#endif

#ifdef USE_vm_readv
	#include <sys/uio.h>
#endif

#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "hack/readline.h"
#include "hack/maps.h"

#ifndef WARNING
	#define WARNING(str, ...) fprintf(stderr, "\033[33mWarning\033[00m: "str"\n", ##__VA_ARGS__)
#endif

#ifndef ERROR
	#define ERROR(str, ...) fprintf(stderr, "\033[31mError\033[00m: "str"\n", ##__VA_ARGS__)
#endif

struct _event;

typedef bool (*callback)(struct _event*, char*);

typedef struct _event {
	pid_t pid;
	int mem_fd;
	Maps *mem;

	bool end;

	callback scan, quit;
} Event;

bool scan(Event *ev, char *in);
bool quit(Event *ev, char *in);
bool interpreting(Event *ev, char *in);

#endif /* end of include guard: HACK_H_326FWU4H */
