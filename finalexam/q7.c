#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
	char buf[32];
	int p[2];
	close( 0 );
	close( 1 );
	pipe( p );
	dup2( 0, 3 );
	dup2( 1, 4 );
	open( "q7.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
	dup2( 2, 6 );
	dup2( 3, 7 );
	dup2( 4, 8 );
	dup2( 5, 9 );
	close( 3 );
	close( 4 );
	close( 5 );
	write(8, "This is a line of text.", 18);
	read(7, buf, 14);
	write(1, "bicycle", 10);
	write(9, "fishsticks", strlen("fishsticks"));
	write(6, buf, 10);
	read(p[0], buf, 32);
	write(6, buf+4, 7);
	/* ... */

	// for (int i = 0; i < 12; i++)
	// 	write(i, "hello", strlen("hello"));
}