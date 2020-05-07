#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

int main() {
	sleep(5);
	close( 0 );
	int rc;
	int p[2];
	pipe( p );
	rc = fork();
	if ( rc == 0 ) {
		rc = write(1, "Child\n", strlen("Child\n"));
	}
	if ( rc > 0 ) {
		rc = fork();
		close(1);
	}
	sleep(5);
	return EXIT_SUCCESS;
}