#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char * argv[])
{
	int fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(1);
	}

	/* let's disable Other's write and execute bit */
	umask(S_IWOTH | S_IXOTH);

	/* O_EXCL: exclusive open means that the file must not exist yet
	 * O_CREAT: create a file, this flag means the mode must be supplied
	 */
	fd = open(argv[1], O_CREAT | O_WRONLY | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);

	if (fd == -1) {
		fprintf(stderr, "Failed create %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}

	close(fd);

	return 0;
}
