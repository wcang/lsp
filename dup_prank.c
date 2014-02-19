#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char * argv[])
{
	int fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s message\n", argv[0]);
		exit(1);
	}

	if ((fd = open("prank.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
		fprintf(stderr, "Failed to open prank.txt for writing: %m\n");
		exit(1);
	}

	dup2(fd, 1);
	close(fd);
	printf("%s\n", argv[1]);

	return 0;
}
