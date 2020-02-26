#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
	int p[2];
	int rc = pipe(p);

	fprintf(stderr, "%d: %d %d\n", getpid(), p[0], p[1]);

	rc = fork();

	if (rc == 0) {
		rc = write(p[1], "RPIirlRPIirlRPIirlRPI", 12);

	} else {
		rc = fork();
		if (rc == 0) {
			rc = write(p[1], "OS-4210:OS-4210:OS-4210", 12);
		}
		char buffer[13];
		rc = read(p[0], buffer, 12);
		buffer[rc] = '\0';
		fprintf(stderr, "%d: [%s]\n", getpid(), buffer);
	}
	return 1;
}