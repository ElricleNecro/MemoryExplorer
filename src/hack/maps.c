#include "hack/maps.h"

void to_first_space(char *str)
{
	while( !isspace(*str) && *str != '\0' )
	{
		str++;
	}
	*str = '\0';
}

char* get_exe_name(const char *link)
{
	unsigned int lb_size = 0;
	char *bin_name = NULL;

	if( ( bin_name = malloc(NAME_BUF_SIZE * sizeof(char)) ) == NULL )
	{
		perror("Allocating 'bin_name'");
		return NULL;
	}

	if( (lb_size = readlink(link, bin_name, NAME_BUF_SIZE)) > 0 )
		bin_name[lb_size] = 0;
	else
		bin_name[0] = 0;

	if( bin_name[0] )
		to_first_space(bin_name);

	return bin_name;
}

bool get_region_type(Maps zone, const char *bin_name, size_t prev_end)
{
	bool is_exe = false;
	static unsigned int regions = 0;

	if( regions > 0 )
	{
		if( (zone)->exec == 'x' || (strncmp((zone)->filename,	bin_name, NAME_BUF_SIZE) != 0 &&
			( (zone)->filename[0] != '\0' || (zone)->start != prev_end ) ) ||
			regions >= 4
		)
		{
			regions = 0;
			is_exe = false;
		} else {
			regions++;
		}
	}
	if( regions == 0 )
	{
		if( (zone)->exec == 'x' && (zone)->filename[0] != '\0' )
		{
			regions++;

			if( strncmp((zone)->filename, bin_name, NAME_BUF_SIZE) == 0 )
				is_exe = true;

			/* strncpy(bin_name, (zone)->filename, NAME_BUF_SIZE); */
			/* bin_name[NAME_BUF_SIZE - 1] = '\0';  [> just to be sure <] */
		}
		/* load_addr = start; */
	}

	/* must have permissions to read and write, and be non-zero size */
	if( ((zone)->write == 'w') && ((zone)->read == 'r') && (((zone)->end - (zone)->start) > (unsigned long int)0) )
	{
		/* determine region type */
		if(is_exe)
			(zone)->type = EXE;
		else if(regions > 0)
			(zone)->type = CODE;
		else if(!strcmp((zone)->filename, "[heap]"))
			(zone)->type = HEAP;
		else if(!strcmp((zone)->filename, "[stack]"))
			(zone)->type = STACK;
		else
			return false;
			/* (zone)->type = _ALL; */
	}
	else
		return false;

	return true;
}

bool Maps_read(struct _maps **zone, pid_t pid)
{
	FILE* fich = NULL;
	char *line = NULL,
	      maps_name[NAME_BUF_SIZE] = {0},
	      bin_link[NAME_BUF_SIZE]  = {0},
	      *bin_name; // [NAME_BUF_SIZE]  = {0};
	size_t lon;
	unsigned long int prev_end = 0x0;

	snprintf(maps_name, NAME_BUF_SIZE, "/proc/%u/maps", pid);

	if( (fich = fopen(maps_name, "r")) == NULL )
	{
		perror(NULL);
		return false;
	}

	snprintf(bin_link, NAME_BUF_SIZE, "/proc/%u/exe", pid);
	bin_name = get_exe_name(bin_link);

	if( bin_name == NULL )
		return false;

	while( getline(&line, &lon, fich) != -1 )
	{
		if( *zone == NULL )
		{
			*zone = malloc(sizeof(struct _maps));
			(*zone)->filename = NULL;
		}
		(*zone)->next = NULL;

		if( (*zone)->filename != NULL )
			free((*zone)->filename);

		if( ((*zone)->filename = malloc(lon * sizeof(char))) == NULL )
		{
			perror("Error reading maps file:");
			fclose(fich);
			return false;
		}

		memset((*zone)->filename, '\0', lon);

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
			(*zone)->filename
		) >= 6 )
		{
			if( get_region_type(*zone, bin_name, prev_end) )
			{
				prev_end = (*zone)->end;
				zone = &(*zone)->next;
			}
			else
			{
				free((*zone)->filename);
				free(*zone);
				*zone = NULL;
			}
		}
		else
		{
			free((*zone)->filename);
			free(*zone);
			*zone = NULL;
		}
	}

	fclose(fich);
	free(line);
	free(bin_name);

	return true;
}

void Maps_free(struct _maps **zone)
{
	struct _maps *actual = *zone;
	while(actual != NULL)
	{
		Maps tmp = actual;
		actual = tmp->next;
		free(tmp->filename);
		free(tmp);
	}
}

