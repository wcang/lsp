#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char * argv[])
{
	pid_t pid;
	int pipefd[2];

	/* create pipe, pipefd[0] is reading end, pipefd[1] is writing end */
	if (pipe(pipefd) == -1) {
		fprintf(stderr, "Paging Mario Brothers. Pipe problem: %m\n");
		exit(1);
	}

	pid = fork();

	switch (pid) {
		case -1:
			fprintf(stderr, "Failed to fork a new process\n");
			exit(1);
		case 0:	/* child process */
			close(pipefd[1]); /* close the writing end of the pipe */
			dup2(pipefd[0], 0); /* close stdin, duplicate reading end of pipe to stdin */
			close(pipefd[0]);  /* it's now okay to close duplicated pipe */
			execlp("wc" , "wc", "-l", (char *) NULL);
			break;
		default: /* parent process */
			close(pipefd[0]); /* close the reading end of the pipe */
			dup2(pipefd[1], 1); /* close stdout, duplicate writing end of pipe to stdout */
			close(pipefd[1]); /* it's now okay to close duplicated pipe */
			execlp("ls", "ls", "-1", (char *) NULL);
			break;
	}

	return 0;
}


