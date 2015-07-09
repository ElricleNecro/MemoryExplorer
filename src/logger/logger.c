#include "logger/logger.h"

static char *def_error = "\033[31m[ERROR]\033[00m ";
static char *def_warn = "\033[33m[WARNING]\033[00m ";
static char *def_debug = "\033[36m[DEBUG]\033[00m ";
static char *def_info = "\033[32m[INFO]\033[00m ";

Logger* Logger_New(FILE *out, const char *name, enum _Level level)
{
	Logger *log = NULL;

	if( (log = malloc(sizeof(struct _Logger))) == NULL )
	{
		perror("Error while allocating the logger");
		return NULL;
	}

	if( (log->name = malloc(sizeof(char) * strlen(name))) == NULL )
	{
		perror("Error while allocating the logger name.");
		free(log);
		return NULL;
	}

	log->output = out;
	log->level  = level;
	strncpy(log->name, name, strlen(name));

	log->error = def_error;
	log->info  = def_info;
	log->debug = def_debug;
	log->warn  = def_warn;

	log->dealloc_error = false;
	log->dealloc_info  = false;
	log->dealloc_debug = false;
	log->dealloc_warn  = false;
	log->dealloc_out   = false;

	return log;
}

void Logger_SetLevel(Logger *log, Level level)
{
	log->level = level;
}

void Logger_Free(Logger *log)
{
	if( log->dealloc_error && log->error != NULL )
		free(log->error);

	if( log->dealloc_warn && log->warn != NULL )
		free(log->warn);

	if( log->dealloc_debug && log->debug != NULL )
		free(log->debug);

	if( log->dealloc_info && log->info != NULL )
		free(log->info);

	if( log->dealloc_out )
		fclose(log->output);

	free(log->name);
	free(log);
}

static int print_with_prompt(FILE *out, const char *ppt, const char *str, va_list va)
{
	int ret;
	size_t s_len = strlen(str), p_len = strlen(ppt);
	char *fmt = NULL;

	if( (fmt = malloc((s_len + p_len + 1) * sizeof(char))) == NULL )
	{
		perror("Error while allocating the logger");
		return -1;
	}

	strcpy(fmt, ppt);
	strncat(fmt, str, s_len);

	ret = vfprintf(
		out,
		fmt,
		va
	);

	free(fmt);
	return ret;
}

int Logger_Error(Logger *log, const char *str, ...)
{
	if( log->level < ERROR )
		return 0;

	int ret;
	va_list va;
	va_start(va, str);

	ret = print_with_prompt(
		log->output,
		log->error,
		str,
		va
	);

	va_end(va);

	return ret;
}

int Logger_Warn(Logger *log, const char *str, ...)
{
	if( log->level < WARN )
		return 0;

	int ret;
	va_list va;
	va_start(va, str);

	ret = print_with_prompt(
		log->output,
		log->warn,
		str,
		va
	);

	va_end(va);

	return ret;
}

int Logger_Debug(Logger *log, const char *str, ...)
{
	if( log->level < DEBUG )
		return 0;

	int ret;
	va_list va;
	va_start(va, str);

	ret = print_with_prompt(
		log->output,
		log->debug,
		str,
		va
	);

	va_end(va);

	return ret;
}

int Logger_Info(Logger *log, const char *str, ...)
{
	if( log->level < INFO )
		return 0;

	int ret;
	va_list va;
	va_start(va, str);

	ret = print_with_prompt(
		log->output,
		log->info,
		str,
		va
	);

	va_end(va);

	return ret;
}

