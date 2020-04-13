#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main( int argc, char * argv[] ) {
	int shmkey = 4000;
	
	int shmid = shmget( shmkey, sizeof( int ), IPC_CREAT | 0660 );

	int * data = shmat( shmid, NULL, 0 );

	int flush=0;

	pid_t pid = fork();

	int i, stop = 4;

	if(pid>0)
		waitpid(pid,NULL,0);

	for(i=1;i<stop;i++)
		*data+=i;

	if ( pid == 0 )
		printf( "Sum is %d\n", *data );

	if (*data > 31)
		flush = 1;

	shmdt( data );

	if (flush)
		shmctl( shmid, IPC_RMID, 0 );

	return EXIT_SUCCESS;
}