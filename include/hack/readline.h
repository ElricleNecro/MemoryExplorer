#ifndef READLINE_H_VHEMJUKD
#define READLINE_H_VHEMJUKD

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <readline/readline.h>
#include <readline/history.h>

typedef struct _rl_data {
	char *line, *prompt, *hist_file;
} RLData;

bool RLData_init(RLData *new, const char *prompt, const char *hist_file);
void RLData_free(RLData *data);
bool RLData_get(RLData *data);
bool RLData_saveHistory(RLData *data);
bool RLData_readHistory(RLData *data);

#endif /* end of include guard: READLINE_H_VHEMJUKD */
