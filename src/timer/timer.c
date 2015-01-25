#include <unistd.h>
#include <stdio.h>

int main(void)
{
	int stop = 5, a = 0;

	fprintf(stderr, "(%d) need a=%d to stop!\n", getpid(), stop);
	fprintf(stderr, "%p, %p\n", &stop, &a);

	while(a != stop) sleep(1);
}
