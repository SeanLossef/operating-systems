#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAXBUF 1024
#define bg 0

char **split(char buffer[MAXBUF], char delim) {
	char *input = calloc(MAXBUF, 1);
	strcpy(input, buffer);

	// Count number of words
	int i = 0;
	int count = 1;
	while (input[i] != '\n' && input[i] != '\0') {
		if (input[i] == delim || input[i] == '\n') {
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

	free(input);

	return output;
}

void free_array(char ** array) {
	char ** ptr = array;
	while (*ptr != NULL) {
		free(*ptr);
		*ptr = NULL;
		ptr++;
	}
}

int main() {
	setvbuf( stdout, NULL, _IONBF, 0 );

	char buffer[MAXBUF];
	struct stat *stats = malloc(sizeof(struct stat));

	int *status = malloc(sizeof(int));
	pid_t pid;

	char * env = getenv("MYPATH");
	if (env == NULL)
		strcpy(buffer, "/bin");
	else
		strcpy(buffer, getenv("MYPATH"));
	char ** mypath = split(buffer, ':');

	while (1) {
		// Check for terminated processes
		pid = waitpid(WAIT_ANY, status, WNOHANG);
		if (pid > 0) {
			if (*status == 0)
				printf("[process %d terminated with exit status %d]\n", pid, *status);
			else
				printf("[process %d terminated abnormally]\n", pid);
		}

		printf("%s$ ", getcwd(buffer, MAXBUF));
		fgets(buffer, MAXBUF, stdin);

		char** tokens;
		char** tokens2;
		tokens = split(buffer, ' ');
		tokens2 = tokens;
		int isPipe = 0;
		int isBg = 0;

		// Find pipe symbol
		char** ptr = tokens;
		while (*ptr != NULL) {
			if (strncmp(*ptr, "|", 1) == 0) {
				free(*ptr);
				*ptr = NULL;
				tokens2 = ptr + 1;
				isPipe = 1;
			} else if (strncmp(*ptr, "&", 1) == 0) {
				free(*ptr);
				*ptr = NULL;
				isBg = 1;
			}
			ptr++;
		}

		// Special Commands
		if (strncmp(tokens[0], "exit", strlen(tokens[0])) == 0) {
			printf("bye\n");
			free_array(tokens);
			free_array(tokens2);
			free(tokens);
			break;
		}
		if (strncmp(tokens[0], "cd", strlen(tokens[0])) == 0) {
			int rc = 0;
			rc = chdir(tokens[1]);
			if (rc == -1)
				fprintf(stderr, "chdir() failed: Not a directory\n");
			free_array(tokens);
			free_array(tokens2);
			free(tokens);
			continue;
		}

		// Fork process to handle command
		pid = fork();
		if (pid) {
			if (isBg) {
				printf("[running background process \"%s\"]\n", tokens[0]);
			} else {
				waitpid(pid, status, 0);
				free_array(tokens);
				free_array(tokens2);
				free(tokens);
			}
		} else {
			int pipefd[2];
			pipe(pipefd);

			if (isPipe && fork()) {
				// Pipe reader
				ptr = mypath;
				while (*ptr != NULL) {
					sprintf(buffer, "%s/%s", *ptr, tokens2[0]);
					if (lstat(buffer, stats) >= 0)
						break;
					ptr++;
				}
				if (*ptr == NULL) {
					fprintf(stderr, "ERROR: command \"%s\" not found\n", tokens2[0]);
					return EXIT_FAILURE;
				}

				if (isPipe)
					dup2(pipefd[0], 0);
				close(pipefd[0]);
				close(pipefd[1]);
				execv(buffer, tokens2);
			} else {
				// Pipe writer
				ptr = mypath;
				while (*ptr != NULL) {
					sprintf(buffer, "%s/%s", *ptr, tokens[0]);
					if (lstat(buffer, stats) >= 0)
						break;
					ptr++;
				}
				if (*ptr == NULL) {
					fprintf(stderr, "ERROR: command \"%s\" not found\n", tokens[0]);
					return EXIT_FAILURE;
				}

				if (isPipe)
					dup2(pipefd[1], 1);
				close(pipefd[0]);
				close(pipefd[1]);
				execv(buffer, tokens);
			}
		}
	}

	free_array(mypath);
	free(mypath);
	free(stats);
	free(status);

	return EXIT_SUCCESS;
}