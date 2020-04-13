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

void * q13( void * arg ) {
	int * s = (int *)arg; *s += 3;
	return NULL;
}

int main() {
	int *x = malloc(sizeof(int)); *x = 5;
	int pid1, pid2;
	pid1 = fork();
	*x = 7;
	if (pid1 != 0)
		pid2 = fork();
	if (pid1 == 0) {
		q13(x);
	} else if (pid2 == 0) {
		q13(x);
	} else {
		*x += 9;
	}

	printf("x = %d\n", *x); free(x);
	int status;
	if (pid1 != 0 && pid2 != 0) {
		waitpid(pid1, &status, 0);
		waitpid(pid2, &status, 0);
	}
	return EXIT_SUCCESS;
}