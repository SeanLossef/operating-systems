#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

unsigned long x = 5;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define CHILDREN 8

void * q7_8( void * arg ) {
	for (int ctr=1; ctr<=10; ctr++) {
		x++;
	}

	pthread_exit( NULL );
}

int main() {
	pthread_t tid[CHILDREN];
	int i;
	for(i=0;i<CHILDREN;i++) {
		int * t = malloc( sizeof( int ) );
		*t = 2 + i*2;
		pthread_create( &tid[i], NULL, q7_8, NULL );
	}
	for(i=0;i<CHILDREN;i++) {
		pthread_join( tid[i], NULL );
	}
	printf("x is %ld\n", x);
	return EXIT_SUCCESS;
}