#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
	int iif, oif;
	ssize_t len;
	unsigned char buf[1500];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s source_file destination_file\n", argv[0]);
		exit(1);
	}

	iif = open(argv[1], O_RDONLY);

	if (iif == -1) {
		fprintf(stderr, "Error opening source %s: %m\n", argv[1]);
		exit(1);
	}

	oif = open(argv[2], O_WRONLY|O_CREAT, S_IRUSR| S_IWUSR);

	if (oif == -1) {
		fprintf(stderr, "Error opening destination file %s: %m\n", argv[2]);
		exit(1);
	}

	while ((len = read(iif, buf, sizeof(buf))) > 0) {
		/* NOTE: potential error, no checking of return value on write */
		write(oif, buf, len);
	}

	if (len < 0) {
		fprintf(stderr, "Error reading file %s: %m\n", argv[1]);
		exit(1);
	}

	close(iif);
	close(oif);
	return 0;

}
