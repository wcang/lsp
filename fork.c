#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

static void child_process(int fd)
{
	int i;
	char buffer[100];

	printf("PID: %d, Parent PID: %d\n", getpid(), getppid());

	for (i = 0; i < 10; ++i) {
		snprintf(buffer, sizeof(buffer), "%d: %d\n", getpid(), i);
		write(fd, buffer, strlen(buffer));
	}

	close(fd);

	if (getpid() % 2) {
		exit(0);
	}
	else {	/* purposely abort to show different behaviour on parent */
		abort();
	}
}


int main(int argc, char * argv[])
{
	int i;
	int fd;
	int status;
	pid_t child_pid[10];

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
			printf("Created child with PID %d\n", child_pid[i]);
		}
	}

	close(fd);
	/* strictly speaking the sleep isn't necessary */
	sleep(2);
	printf("=================================================\n");	

	for (i = 0; i < 10; ++i) {
		waitpid(child_pid[i], &status, 0);

		if (WIFEXITED(status)) {
			printf("Child %d exited with status %u\n", child_pid[i], WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status)) {
			printf("Child %d killed by signal %d\n", child_pid[i], WTERMSIG(status));
		}
	}

	return 0;
}

