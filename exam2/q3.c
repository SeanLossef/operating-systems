#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

void * q3(void * y) {
	int * x = (void*) y;
	char * s = calloc( *x + 16, sizeof(char));
	sprintf(s, "ABCDEFGH");
	fprintf(stderr, "%s", s);
	return NULL;
}

int main() {
	int * z = calloc(1, sizeof(int));
	if (fork() == 0) *z = 10;
	pthread_t t1;
	pthread_create(&t1, NULL, q3, z);
	fprintf(stderr, "ERROR");
	pthread_join(t1, NULL);
	return EXIT_SUCCESS;
}