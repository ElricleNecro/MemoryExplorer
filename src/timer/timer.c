#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char **argv)
{
	/* printf("argc == %d\n", argc); */
	/* for(int i=0; i<argc; i++) */
		/* printf("%d -> '%s'\n", i, argv[i]); */

	int stop = 5, a = 0;
	double c = 60.;
	double *d = malloc(sizeof(double));

	*d = 5.;

	fprintf(stderr, "(%d) need a=%d to stop!\n", getpid(), stop);
	fprintf(stderr, "&stop = %p, &a = %p, &c = %p\n", &stop, &a, &c);
	fprintf(stderr, "stop = %d, a = %d, c = %g\n", stop, a, c);
	fprintf(stderr, "&d = %p, d = %p, *d = %g\n", &d, d, *d);

	while(a != stop) sleep(1);

	fprintf(stderr, "%g, %d, %d\n", *d, a,stop);

	free(d);

	return 0;
}

