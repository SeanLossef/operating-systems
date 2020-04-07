#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void * sonny( void * arg ) {
	int * s = (int *)arg;
	printf( "%ld lucky %d\n", pthread_self(), *s ); return NULL;
}

int main() {
	pthread_t tid1, tid2;
	int x = 13;
	int rc = pthread_create( &tid1, NULL, sonny, &x );
	x = 7;
	rc = pthread_join( tid1, NULL );
	rc = pthread_create( &tid2, NULL, sonny, &x );
	printf( "%d unlucky %d\n", rc, x );
	rc = pthread_join( tid2, NULL );
	return EXIT_SUCCESS;
}