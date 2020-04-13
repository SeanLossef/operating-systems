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

void * q14( void * args ) {
	int t = *(int *)args;
	char buffer[32];
	int rc = read( t, buffer, 9 );
	sleep(t);
	buffer[rc] = '\0';
	printf( "%s", buffer );
	return NULL;
}

int main() {
	char * s = "CUCKOO FOR COCOA PUFFS";
	int p[2];
	close( 2 );
	close( 0 );
	pipe( p );
	write( p[1], s, 11 );
	pid_t pid = fork();
	pthread_t t1, t2, t3, t4;
	if ( pid == 0 ) {
		fprintf( stderr, "%s", s );
		printf( "I'M " );
		pthread_create( &t1, NULL, q14, &p[0] );
		pthread_create( &t2, NULL, q14, &p[0] );
		pthread_join( t1, NULL );
		pthread_join( t2, NULL );
	} else /* pid > 0 */ {
		wait( NULL );
		pthread_create( &t3, NULL, q14, &p[0] );
		pthread_create( &t4, NULL, q14, &p[0] );
		pthread_join( t3, NULL );
		pthread_join( t4, NULL );
	}
	return EXIT_SUCCESS;
}