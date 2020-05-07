#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	char c = 'C';
	unsigned char d = 0;
	for (int ctr=0; ctr<512+c+10; ctr++)
		d++;
	printf( "<%c>\n", d );
}