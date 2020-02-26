#include <unistd.h>
#include <fcntl.h>

int main() {
	int fd = open("q11.txt", O_WRONLY | O_CREAT | O_TRUNC, 0766);
	write(fd, "First", 5);
	close(fd);
	fd = open("q11.txt", O_WRONLY | O_CREAT, 0766);
	write(fd, "Next", 3);
	close(fd);
}