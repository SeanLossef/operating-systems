#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#define MAXBUF 1024
#define bg 0

char **split(char buffer[MAXBUF]) {
	char *input = calloc(MAXBUF, 1);
	strcpy(input, buffer);

	// Count number of words
	int i = 0;
	int count = 1;
	while (input[i] != '\n') {
		if (input[i] == ' ' || input[i] == '\n') {
			input[i] = '\0';
			count++;
		}
		i++;
	}
	input[i] = '\0';

	// Create array
	char **output = malloc((count + 1) * sizeof(char*));
	output[count] = NULL;

	// Copy tokens indivually into array
	int j = 0;
	for (int k = 0; k < count; k++) {
		int len = strlen(input + j);
		output[k] = calloc(len + 1, 1);
		strcpy(output[k], input + j);
		j += len + 1;
	}

	return output;
}

int main() {
	char buffer[MAXBUF];
	struct stat *stats = malloc(sizeof(struct stat));

	while (1) {
		printf("%s$ ", getcwd(buffer, MAXBUF));
		fgets(buffer, MAXBUF, stdin);

		char** tokens;
		tokens = split(buffer);

		sprintf(buffer, "/bin/%s", tokens[0]);
		if (lstat(buffer, stats) < 0) {
			printf("ERROR: command \"%s\" not found\n", tokens[0]);
		} else {
			pid_t pid = fork();
			if (pid) {
				int *status = NULL;
				if (bg) {
					printf("[running background process \"\"]\n");
				} else {
					waitpid(pid, status, 0);
				}
			} else {
				execv(buffer, tokens);
			}
		}

		
	}

	return EXIT_SUCCESS;
}