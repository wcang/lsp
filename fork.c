#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static void child_process(int fd)
{
	int i;
	char buffer[100];

	printf("PID: %u, Parent PID: %u\n", getpid(), getppid());

	for (i = 0; i < 10; ++i) {
		snprintf(buffer, sizeof(buffer), "%u: %d\n", getpid(), i);
		write(fd, buffer, strlen(buffer));
	}

	close(fd);
	exit(0);
}


int main(int argc, char * argv[])
{
	int i;
	int fd;
	int child_pid[10];

	fd = open("child.log", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		fprintf(stderr, "Hi there... something wrong with opening a log file for your offspring: %s\n", strerror(errno));
		exit(1);
	}

	for (i = 0; i < 10; ++i) {
		child_pid[i] = fork();

		if (child_pid[i] == 0) { /* I'm the child, let do something else */
			child_process(fd);
		}
		else if (child_pid[i] == -1) {
			fprintf(stderr, "Oops... Labour pain. Cannot give birth to child: %s\n", strerror(errno));
			exit(1);
		}
		else { /* parent proceeds with the loop */
			printf("Created child with PID %u\n", child_pid[i]);
		}
	}

	close(fd);
	pause();
	return 0;
}

