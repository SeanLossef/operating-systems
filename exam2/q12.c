#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>

void * q12( void * arg ) {
	int * s = (int *)arg;
	int p = *s;
	sleep(2);
	*s = p + 3;
	return NULL;
}

int main() {
	pthread_t tid1, tid2;
	int x = 5;
	pthread_create( &tid1, NULL, q12, &x );
	pthread_create( &tid2, NULL, q12, &x );
	sleep(1);
	x = 9;
	pthread_join( tid1, NULL );
	pthread_join( tid2, NULL );

	printf("x is : %d\n", x);

	return EXIT_SUCCESS;
}