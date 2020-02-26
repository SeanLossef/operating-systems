#include <unistd.h>
#include <stdio.h>

int main() {
	int ctr = 0;
	while (ctr < 4)
		ctr += fork() + 1;

	printf("This is a PROCESS!\n");

	return 0;
}