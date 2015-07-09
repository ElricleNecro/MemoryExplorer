#ifndef MAPS_H_RIAU46YX
#define MAPS_H_RIAU46YX

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

#ifndef NAME_BUF_SIZE
	#define NAME_BUF_SIZE 128
#endif

typedef enum _region_type {
	EXE,
	CODE,
	HEAP,
	STACK,
	_ALL
} RegionType ;

typedef struct _maps {
	char read, write, exec, cow;
	unsigned long int start, end;
	int offset, dev_major, dev_minor, inode;
	char *filename;

	RegionType type;

	struct _maps *next;
}* Maps;

bool Maps_read(Maps *zone, pid_t pid);
void Maps_free(Maps *zone);

#endif /* end of include guard: MAPS_H_RIAU46YX */

