#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static void child_process()
{
	printf("PID: %d, Parent PID: %d\n", getpid(), getppid());
	sleep(4);
	printf("PID: %d, Parent PID: %d. I've been adopted\n", getpid(), getppid());
	exit(0);
}


int main(int argc, char * argv[])
{
	pid_t child_pid;

	child_pid = fork();

	if (child_pid == 0) { /* I'm the child, let do something else */
		child_process();
	}
	else if (child_pid == -1) {
		fprintf(stderr, "Oops... Labour pain. Cannot give birth to child: %s\n", strerror(errno));
		exit(1);
	}
	else {
		printf("I am suffering Batman's parents fate. Goodbye my child %u\n", child_pid);
	}

	sleep(1);

	return 0;
}

