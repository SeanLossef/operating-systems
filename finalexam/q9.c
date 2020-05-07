#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
	size_t size = 2;
	int *x = malloc(size * sizeof(int));
	char val = 'c';
	for (int ctr=0; ctr<(1<<15);ctr++) {
		if (ctr >= size) {
			size *= 2;
			val++;
			realloc(x, size * sizeof(int));
		}
		*(x+ctr) = val;
		printf("%c\n", val);
	}
	return EXIT_SUCCESS;
}