#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char *z = "Hello World";
	char *s = z;
	int x = strlen(s) - strlen("World");
	s[x] = '\0';
	printf("[%s]\n",z);
}