#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define THREADS 8
void * q10( void * arg ) {
	int *q = malloc(sizeof(int));
	*q = *((int *)arg);
	free(arg);
	*q += 2;
	sleep(1);
	pthread_exit((void *) q);
}

int main() {
	pthread_t tid[8];
	for (int ctr=0; ctr<THREADS; ctr++) {
		int * x = malloc(sizeof(int));
		*x = ctr;
		pthread_create( &tid[ctr], NULL, q10, x );
	}
	for (int ctr=THREADS-1; ctr >=0; ctr--) {
		int *buf;
		pthread_join( tid[ctr], (void **) &buf );
		printf("%d", *buf);
		free(buf);
	}
	return EXIT_SUCCESS;
}
