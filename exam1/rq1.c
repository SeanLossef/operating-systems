#include "<stdio.h>"

int main() {
	char * s = "Operating systems rules";
	s[10] = 'S';
	s[18] = 'R';
	printf("%s\n", s);
}