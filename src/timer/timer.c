#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int stop = 5, a = 0;
	double *d = malloc(sizeof(double));

	*d = 5.;

	fprintf(stderr, "(%d) need a=%d to stop!\n", getpid(), stop);
	fprintf(stderr, "&stop = %p, &a = %p\n", &stop, &a);
	fprintf(stderr, "stop = %d, a = %d\n", stop, a);
	fprintf(stderr, "&d = %p, d = %p, *d = %g\n", &d, d, *d);

	while(a != stop) sleep(1);

	fprintf(stderr, "%g, %d, %d\n", *d, a,stop);

	free(d);

	return 0;
}
