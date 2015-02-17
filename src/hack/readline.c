#include "hack/readline.h"

bool RLData_init(RLData *new, const char *prompt, const char *hist_file)
{
	size_t l_prompt = strlen(prompt),
	       l_hist_file = strlen(hist_file);

	new->prompt = NULL;
	new->line = NULL;								// This one HAVE to stay at NULL before the get.
	new->hist_file = NULL;

	if( prompt )									// If prompt is not NULL:
	{
		if( (new->prompt = calloc(l_prompt + 1, sizeof(char))) == NULL )	// We allocate our string.
		{									// Dealing with possible errors.
			perror("Prompt allocation failed:");
			RLData_free(new);
			return false;
		}
		strncpy(new->prompt, prompt, l_prompt);					// If no errors, we copy it.
	}

	if( hist_file )									// Exactly the same story here.
	{
		if( (new->hist_file = calloc(l_hist_file + 1, sizeof(char))) == NULL )
		{
			perror("History file name allocation failed:");
			RLData_free(new);
			return false;
		}
		strncpy(new->hist_file, hist_file, l_hist_file);
	}

	return true;
}

void RLData_free(RLData *data)
{
	if( data->hist_file )
		free(data->hist_file), data->hist_file = NULL;

	if( data->prompt )
		free(data->prompt), data->prompt = NULL;

	if( data->line )
		free(data->line), data->line = NULL;
}

bool RLData_get(RLData *data)
{
	if( data->line )			// Is line has already been allocated...
	{
		free(data->line);		// We free it!
		data->line = (char *)NULL;	// Without forgetting to put it to NULL.
	}

	data->line = readline(data->prompt);	// Then we reading user input, letting the function
						// allocating what is needed for the line.

	if( data->line && *data->line )		// Checking if the line has been allocated and is not empty
		add_history(data->line);	// and add it to the history.

	return true;
}

bool RLData_saveHistory(RLData *data)
{
	if( write_history(data->hist_file) )	// We save the history in the appropriate file. If the return is different of 0
		return false;			// There is a problem.
	return true;
}

bool RLData_readHistory(RLData *data)
{
	if( read_history(data->hist_file) )	// We read the history file. The return  is the same as the write_history.
		return false;
	return true;
}

