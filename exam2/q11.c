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

#define MAXBUFFER 8

int main() {
	int flag = 0;
	char buffer[ MAXBUFFER + 1 ];
	int sd = socket( AF_INET, SOCK_DGRAM, 0 );
	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl( INADDR_ANY );
	server.sin_port = htons( 8128 );

	bind( sd, (struct sockaddr *) &server, sizeof( server ) ); struct sockaddr_in client;
	int n, len = sizeof( client );
	while ( 1 ) {
		n = recvfrom( sd, buffer, MAXBUFFER, 0, (struct sockaddr *) &client, (socklen_t *) &len );
		if ( n > 0 ) {
			flag = (flag + n) % 17; if (flag == 7 )
			{
				sendto( sd, "Okay, enough!", 13, 0, (struct sockaddr *) &client, len );
			}
		}
		else return EXIT_FAILURE;
	}
}