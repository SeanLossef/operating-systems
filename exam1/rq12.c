#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	int t = 19, u = 70;
	int * v = malloc(sizeof(int));
	int * w = calloc(t, sizeof(int));
	int * x = NULL;

	*v = t;
	free(w);
	w = x;
	x = v;
	fprintf(stderr, "%d %d\n", *v, *w);

	*x = 73;
	w = &u;
	fprintf(stderr, "%d %d\n", *x, *w);

	free(v);
	return 1;
}