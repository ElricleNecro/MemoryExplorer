#ifndef HACK_H_326FWU4H
#define HACK_H_326FWU4H

#if defined(USE_PTRACE) && defined(USE_vm_readv) // {
	#undef USE_vm_readv
// }
#elif !defined(USE_PTRACE) && !defined(USE_vm_readv) // {
	#define USE_PTRACE
#endif // }

#if defined(USE_vm_ready) && (defined(__APPLE__) && defined(__MACH__)) // {
	#define USE_PTRACE
	#undef USE_vm_ready
#endif // }

#ifdef USE_PTRACE // {
	#include <sys/wait.h>
	#include <sys/types.h>
	#include <sys/ptrace.h>
// }
#elif defined(USE_vm_readv) // {
	#include <sys/uio.h>
#endif // }

#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __linux__ // {
	#include <byteswap.h>
// }
#elif defined(__APPLE__) && defined(__MACH__) // {
	static inline unsigned short bswap_16(unsigned short x);
	static inline unsigned int bswap_32(unsigned int x);
	static inline unsigned long long bswap_64(unsigned long long x);
#endif // }

#include "logger/logger.h"
#include "hack/maps.h"
#include "dict/dict.h"

#if defined(USE_PTRACE) && (defined(__APPLE__) && defined(__MACH__)) // {
	#define PTRACE_ATTACH PT_ATTACH
	#define PTRACE_DETACH PT_DETACH
	#define PTRACE_TRACEME PT_TRACE_ME
	#define PTRACE_CONT PT_CONTINUE

	#define PTRACE_PEEKDATA PT_READ_D
	#define PTRACE_POKEDATA PT_WRITE_D
#endif // }

#ifndef WARNING // {
	#define WARNING(str, ...) fprintf(stderr, "\033[33mWarning\033[00m: "str"\n", ##__VA_ARGS__)
#endif // }

#ifndef ERROR // {
	#define ERROR(str, ...) fprintf(stderr, "\033[31mError\033[00m: "str"\n", ##__VA_ARGS__)
#endif // }

// two's complement:
#define cad(var) ((~var) + 1)

struct _event;

typedef bool (*callback)(struct _event*, char*);

typedef struct _event {
	// PID:
	pid_t pid;

	// Memory file identifier:
	int mem_fd;

	// Memory mapping of the process:
	Maps mem;

	// Logger specific to the process:
	Logger *log;

	// Command and arguments:
	char **args;
	int nb_args;
} Event;

Event* Event_New(pid_t pid);
Event* Event_NewFromCmd(const int nb_args, const char **args);
void Event_Free(Event *ev);

void Event_SetPID(Event *ev, pid_t pid);
void Event_Launch(Event *ev);
void Event_ReadMap(Event *ev);
bool Event_PrintMap(Event *ev);
#ifdef USE_PTRACE // {
bool Event_Attach(Event *ev);
bool Event_Detach(Event *ev);
#endif // }

bool Event_Scan(Event *ev, size_t offset, ssize_t bytes_to_read, void *out);
bool Event_Write(Event *ev, size_t offset, ssize_t bytes_to_read, void *in);

#endif /* end of include guard: HACK_H_326FWU4H */
