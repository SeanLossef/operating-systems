#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAXBUFFER 6
int main() {
	char buffer[ MAXBUFFER + 1 ];
	int sd = socket( PF_INET, SOCK_DGRAM, 0 );

	struct sockaddr_in server;

	server.sin_family = PF_INET;
	server.sin_addr.s_addr = htonl( INADDR_ANY );
	server.sin_port = htons( 8888 );

	bind( sd, (struct sockaddr *)&server, sizeof( server ) ); 
	struct sockaddr_in client;

	int i, n;
	socklen_t len = sizeof( client );

	do {
		n = recvfrom( sd, buffer, MAXBUFFER, 0, (struct sockaddr *) &client, &len );
		fprintf( stderr, "<" );
		for ( i = 0 ; i < n ; i += 2 )
			fprintf( stderr, "%c", buffer[i] );
		fprintf( stderr, ">" );
	}
	while ( n > 0 );
	return EXIT_SUCCESS;
}