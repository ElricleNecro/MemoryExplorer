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

#include "logger/logger.h"
#include "hack/maps.h"
#include "dict/dict.h"

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
	Logger *log;

	bool quit;

	unsigned long _addr;
} Event;

bool scan(Event *ev, size_t offset, ssize_t bytes_to_read);
bool quit(Event *ev);
bool print_map(Event *ev);

#endif /* end of include guard: HACK_H_326FWU4H */
