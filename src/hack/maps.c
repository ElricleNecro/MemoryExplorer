#include "hack/maps.h"

bool Maps_read(Maps **zone, pid_t pid)
{
	FILE* fich = NULL;
	char *line = NULL,
	      maps_name[NAME_BUF_SIZE] = {0},
	      bin_link[NAME_BUF_SIZE]  = {0},
	      bin_name[NAME_BUF_SIZE]  = {0};
	bool is_exe = false;
	size_t lon;
	unsigned int regions = 0, lb_size = 0;
	unsigned long int prev_end = 0x0;

	snprintf(maps_name, NAME_BUF_SIZE, "/proc/%u/maps", pid);

	if( (fich = fopen(maps_name, "r")) == NULL )
	{
		perror(NULL);
		return false;
	}

	snprintf(bin_link, NAME_BUF_SIZE, "/proc/%u/exe", pid);
	if( ( lb_size = readlink(bin_link, bin_name, NAME_BUF_SIZE) ) > 0 )
	{
		bin_name[lb_size] = 0;
	} else {
		bin_name[0] = 0;
	}

	while( getline(&line, &lon, fich) != -1 )
	{
		if( *zone == NULL )
			*zone = malloc(sizeof(struct _maps));
		(*zone)->next = NULL;

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
			if( regions > 0 )
			{
				if(
					(*zone)->exec == 'x' ||
					(
						strncmp(
							(*zone)->filename,
							bin_name,
							NAME_BUF_SIZE
						) != 0 &&
						(
							(*zone)->filename[0] != '\0' ||
							(*zone)->start != prev_end
						)
					) ||
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
				if(
					(*zone)->exec == 'x' &&
					(*zone)->filename[0] != '\0'
				)
				{
					regions++;
					if(
						strncmp(
							(*zone)->filename,
							bin_name,
							NAME_BUF_SIZE
						) == 0
					)
						is_exe = true;
					strncpy(bin_name, (*zone)->filename, NAME_BUF_SIZE);
					bin_name[NAME_BUF_SIZE - 1] = '\0';  /* just to be sure */
				}
				/* load_addr = start; */
			}
			prev_end = (*zone)->end;

			/* must have permissions to read and write, and be non-zero size */
			if( ((*zone)->write == 'w') && ((*zone)->read == 'r') && (((*zone)->end - (*zone)->start) > (unsigned long int)0) )
			{
				/* determine region type */
				if(is_exe)
					(*zone)->type = EXE;
				else if(regions > 0)
					(*zone)->type = CODE;
				else if(!strcmp((*zone)->filename, "[heap]"))
					(*zone)->type = HEAP;
				else if(!strcmp((*zone)->filename, "[stack]"))
					(*zone)->type = STACK;

			}
			/*************************************************************************************************/
			/* if( regions > 0 ) */
			/* { */
				/* if( */
					/* exec == 'x' || */
					/* ( */
						/* strncmp(fname, ) */
					/* ) */
				/* ) */
			/* } */
			/* if( (*zone)->read == 'r' && (*zone)->write == 'w' && ((*zone)->end - (*zone)->start) > 0 ) */
			/* { */
				/* if( !strcmp(fname, "[heap]") ) */
					/* (*zone)->type = HEAP; */
				/* else if( !strcmp(fname, "[stack]") ) */
					/* (*zone)->type = STACK; */
				/* else if( ( (*zone)->exec == 'x' && fname != '\0' ) && */
					 /* ( strncmp(fname, exename, NAME_BUF_SIZE) == 0 ) */
				/* ) */
				/* { */
					/* (*zone)->type = EXE; */
				/* } */
			/* } */
			/*************************************************************************************************/

			zone = &(*zone)->next;
		}
		else
		{
			free(*zone);
		}
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

