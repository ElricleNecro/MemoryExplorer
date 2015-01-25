#include "hack/maps.h"

bool Maps_read(Maps **zone, pid_t pid)
{
	FILE* fich = NULL;
	size_t lon;
	char *line = NULL;
	char maps_name[NAME_BUF_SIZE] = {0};
	snprintf(maps_name, NAME_BUF_SIZE, "/proc/%u/maps", pid);

	if( (fich = fopen(maps_name, "r")) == NULL )
	{
		perror(NULL);
		return false;
	}

	while( getline(&line, &lon, fich) != 1 )
	{
		char *fname = NULL;

		if( (fname = malloc(lon * sizeof(char))) == NULL )
		{
			perror("Error reading maps file:");
			fclose(fich);
			return false;
		}

		if( *zone == NULL )
			*zone = malloc(sizeof(struct _maps));

		if( sscanf(
			line,
			"%lx-%lx %c%c%c%c %x %x:%x %u %s",
			&(*zone)->start,
			&(*zone)->end,
			&(*zone)->read,
			&(*zone)->write,
			&(*zone)->exec,
			&(*zone)->cow,
			&(*zone)->offset,
			&(*zone)->dev_major,
			&(*zone)->dev_minor,
			&(*zone)->inode,
			fname
		) >= 6 )
		{
			if( (*zone)->read == 'r' && (*zone)->write == 'w' && ((*zone)->end - (*zone)->start) > 0 )
		}

		free(fname);
	}

	fclose(fich);

	return true;
}

void Maps_free(Maps **zone)
{
	Maps *actual = *zone;
	while(actual != NULL)
	{
		Maps *tmp = actual;
		actual = tmp->next;
		free(tmp);
	}
}

