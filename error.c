#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char * argv[])
{
	int fd;

	fd = open("non-existent-file.txt", O_RDONLY);

	if (fd == -1) {
		printf("Error opening file: %s\n", strerror(errno));
	}

	if (close(fd) == -1) {
		printf("Error closing file: %m\n");
	}

	return 0;
}
