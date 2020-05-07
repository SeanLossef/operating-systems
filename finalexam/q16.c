#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <pthread.h>

void * q16( void * arg ) {
	int * q = (int *)arg;
	if (*q == 16)
		sleep(1);
	printf("x is %d\n", *q);
	*q -= 6;
	printf( "LUCKY %d\n", *q );
	return NULL;
}

int main() {
	printf("%d %d %d\n", EAGAIN, EINVAL, EPERM);
	pthread_t tid1, tid2;
	int x = 16;
	int rc = pthread_create( &tid1, NULL, q16, &x );
	rc = pthread_create( &tid2, NULL, q16, &rc );
	sleep(2);
	x = 13;
	rc = pthread_join( tid1, NULL );
	rc = pthread_join( tid2, NULL );
	return EXIT_SUCCESS;
}