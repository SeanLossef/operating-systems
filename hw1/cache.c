#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_WORD 128

int hash(char *word, int cachesize) {
	int sum = 0;
	while (*word != '\0') {
		sum += (int)(*word);
		word++;
	}
	return sum % cachesize;
}

void cache_set(char **cache, int h, char *val) {
	*(cache + h) = val;
}

int main(int argc, char **argv) {

	if (argc != 3) {
		fprintf(stderr, "ERROR: Not enough args\n");
		return EXIT_FAILURE;
	}

	int cachesize = atoi(*(argv + 1));
	if (cachesize <= 0) {
		fprintf(stderr, "ERROR: Invalid cache length\n");
		return EXIT_FAILURE;
	}
	char *filename = *(argv + 2);

	char **cache = calloc(cachesize, sizeof(char*));

	// Read file
	FILE *file = fopen(filename, "r");
	char *word = malloc(MAX_WORD);
	size_t n = 0;
	int c;

	if (file == NULL) {
		fprintf(stderr, "ERROR: Could not open file\n");
		return EXIT_FAILURE;
	}

	// Loop through all chars in file
	while ((c = fgetc(file)) != EOF) {
		
		// Check if word is done
		if (!isalpha((char) c) && !isdigit((char) c)) {

			if (n >= 3) {
				*(word + n) = '\0';
				int h = hash(word, cachesize);

				if (*(cache + h) == NULL) {
					cache_set(cache, h, calloc(strlen(word)+1, sizeof(char)));
					strcpy(*(cache + h), word);
					printf("Word \"%s\" ==> %d (calloc)\n", word, h);
				} else {
					cache_set(cache, h, realloc(*(cache + h), strlen(word)+1));
					strcpy(*(cache + h), word);
					printf("Word \"%s\" ==> %d (realloc)\n", word, h);
				}
			}
			
			n = 0;
		} else {
			*(word + n) = (char) c;
			n++;
		}
	}

	// Print and free final cache state
	for (int i = 0; i < cachesize; i++) {
		if (*(cache + i) != NULL) {
			printf("Cache index %d ==> \"%s\"\n", i, *(cache + i));
			free(*(cache + i));
		}
	}
	free(cache);
	free(word);
	fclose(file);

	return EXIT_SUCCESS;
}