#ifndef LOGGER_H_7FKXRP9Q
#define LOGGER_H_7FKXRP9Q

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum _Level {
	ERROR=0,
	WARN,
	INFO,
	DEBUG,
	ALL
} Level;

typedef struct _Logger {
	FILE *output;
	enum _Level level;

	bool dealloc_out,
	     dealloc_error,
	     dealloc_warn,
	     dealloc_debug,
	     dealloc_info;

	char *error,
	     *warn,
	     *debug,
	     *info,
	     *name;
} Logger;

Logger* Logger_New(FILE *out, const char *name, enum _Level level);
void Logger_Free(Logger *log);

void Logger_SetLevel(Logger *log, Level level);

int Logger_Info(Logger *log, const char *str, ...) __attribute__ ((format (printf, 2, 3)));
int Logger_Error(Logger *log, const char *str, ...) __attribute__ ((format (printf, 2, 3)));
int Logger_Warn(Logger *log, const char *str, ...) __attribute__ ((format (printf, 2, 3)));
int Logger_Debug(Logger *log, const char *str, ...) __attribute__ ((format (printf, 2, 3)));

#endif /* end of include guard: LOGGER_H_7FKXRP9Q */

