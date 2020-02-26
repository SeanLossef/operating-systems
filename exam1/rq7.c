#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char * s = calloc(100, sizeof(char));
	strcpy(s, "RENSSELAER POLYTECHNIC");
	char * r = s + 20;
	strcpy(r+3, "INSTITUTE");
	char * q = r - 5;
	printf("[%c][%c][%c][%s]\n", *(s+9), *(q-4), *(r+3), s);
	return 1;
}