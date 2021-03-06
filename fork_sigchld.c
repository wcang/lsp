#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>	/* for sigaction */
#include <sys/wait.h>	/* for wait syscall */


/* Simple demo to show how to handle death of child using signal handling
 * DO NOTE that program is not ideal because parent process may die before
 * child dies. For a better example, please refer to fork.c 
 */

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
	exit(0);
}


static void signal_handler(int signo)
{
	pid_t pid;
	int status;

	if (signo == SIGCHLD) {
		pid = wait(&status);

		if (WIFEXITED(status)) {
			printf("Child %d exited with status %u\n", pid, WEXITSTATUS(status));
		}
	}
}


int main(int argc, char * argv[])
{
	int i;
	int fd;
	pid_t child_pid[10];
	struct sigaction sa;
	
	/* zero the structure so that no garbage value is passed to sigaction */
	memset(&sa, 0, sizeof(sa)); 
	sa.sa_handler = signal_handler;
	/* we want to install signal handler for SIGCHLD. Not interested in
	 * the old (default) signal handler for SIGCHLD
	 */
	sigaction(SIGCHLD, &sa, NULL);

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
	pause();
	return 0;
}

