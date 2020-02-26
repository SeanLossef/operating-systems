#include <stdlib.h>

int main() {
	char * c[20];
	char * s = calloc(10, sizeof(int));
	c[15] = s;
}