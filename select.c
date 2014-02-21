#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h> /* this is needed for struct sockaddr_in (IPv4 address) */
#include <signal.h> 	/* signal handling */
#include <unistd.h>	/* close syscall */
#include <arpa/inet.h>	/* for inet_ntop */
#include <stdbool.h>	/* boolean for C99 */


volatile sig_atomic_t term = 0;


static int setup_server(const char * caddr, uint16_t port)
{
	int sockfd;
	int yes = 1;
	struct sockaddr_in addr;

	/* AF_INET: IPv4 protocol
	 * SOCK_STREAM: TCP protocol
	 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Socket creation failed: %s\n", strerror(errno));
		exit(1);
	}

	/* reuse address to ensure that bind doesn't fail */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		fprintf(stderr, "Enabling address reuse failed: %m\n");
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	/* NOTE: inet_pton returns 1 on success. This differs from syscall */
	if (inet_pton(AF_INET, caddr, &addr.sin_addr) == 0) {
		fprintf(stderr, "Conversion of IP address %s failed.\n", caddr);
		exit(1);
	}

	/* bind address:port to our socket */
	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Failed to bind socket to %s:%u : %m\n",
			       caddr, port);
		exit(1);
	}

	/* set our socket to listening state with maximum of 5 backlog connections */
	if (listen(sockfd, 5) == -1) {
		fprintf(stderr, "Failed to listen to socket: %m\n");
		exit(1);
	}

	return sockfd;
}


/* does nothing useful, simply print how many bytes received from client.
 * if there is an error or the socket is closed, returns -1
 * otherwise returns 0
 */
int process_client(int client_fd)
{
	int len;
	char data[256];

	len = recv(client_fd, data, sizeof(data), 0);

	if (len == -1) {
		fprintf(stderr, "reception of TCP data failed: %m\n");
		return -1;
	}
	else if (len == 0) {
		printf("socket %d is closed\n", client_fd);
		return -1;
	}

	printf("Received %d bytes of data from socket %d\n", len, client_fd);
	return 0;
}


static void signal_handler(int signum)
{
	if (signum == SIGTERM) {
		printf("Terminate\n");
		term = 1;
	}
}



static void install_sighandler(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;
	//sa.sa_flags = SA_RESTART;
	
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		fprintf(stderr, "Unable to install signal handler for SIGTERM: %m\n");
		exit(1);
	}
}

/* mark file descriptor into unused slot (the one with -1) of the array
 * Very primitive. Real world use case should dynamically adjust array size.
 * This one simply close the socket if the array is full if no empty
 * slot to use and return false
 */
static bool mark_used(int * fds, int client_fd, unsigned int size)
{
	unsigned int i;
	bool marked = false;

	for (i = 0; i < size; ++i) {
		if (fds[i] == -1) {
			fds[i] = client_fd;
			marked = true;
			break;
		}	
	}

	if (!marked) {
		close(client_fd);
	}

	return marked;
}


/* like mark_used, but doing the opposite */
static void mark_unused(int * fds, int client_fd, unsigned int size)
{
	unsigned int i;

	for (i = 0; i < size; ++i) {
		if (fds[i] == client_fd) {
			fds[i] = -1;
			break;
		}	
	}
}


int main(int argc, char * argv[])
{
	int server_fd, client_fd;
	unsigned long port;
	struct sockaddr_in peer_addr;
	char * caddr, * endptr;
	char cpeer_addr[20];
	socklen_t addr_len;
	int fds[20];
	fd_set read_fds, master_fds;
	int maxfd;
	unsigned int loop;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s address port_number\n", argv[0]);
		exit(1);
	}

	caddr = argv[1];
	port = strtoul(argv[2], &endptr, 10);

	if (endptr[0] != 0 || port > UINT16_MAX) {
		fprintf(stderr, "Invalid argument %s for port number\n", argv[2]);
		exit(1);
	}

	install_sighandler();
	server_fd = setup_server(caddr, port);

	/* empty file descriptor set */
	FD_ZERO(&master_fds);
	memset(fds, -1, sizeof(fds));
	
	/* we want to get the read event for accepting socket */
	FD_SET(server_fd, &master_fds);
	maxfd = server_fd;

	while (!term) {
		read_fds = master_fds;

		if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
			fprintf(stderr, "Error in select syscall: %m\n");
			continue;
		}

		/* there is a new event for our listening socket */
		if (FD_ISSET(server_fd, &read_fds)) {
			memset(&peer_addr, 0, sizeof(peer_addr));
			addr_len = sizeof(peer_addr);
			client_fd = accept(server_fd, (struct sockaddr *) &peer_addr, &addr_len);

			if (client_fd == -1) {
				fprintf(stderr, "Failed to accept TCP connection: %m\n");
			}
			else {
				inet_ntop(AF_INET, &peer_addr.sin_addr, cpeer_addr, sizeof(cpeer_addr));
				printf("Receive connection from %s:%u\n", cpeer_addr, ntohs(peer_addr.sin_port));
				if (mark_used(fds, client_fd, sizeof(fds) / sizeof(fds[0]))) {
					FD_SET(client_fd, &read_fds);
					FD_SET(client_fd, &master_fds);
					
					if (maxfd < client_fd)
						maxfd = client_fd;
				}
				else {
					printf("Client connection closed due to empty slot\n");
				}
			}
		}
		
		for (loop = 0; loop < sizeof(fds) / sizeof(fds[0]) && !term; ++loop) {
			client_fd = fds[loop];

			if (FD_ISSET(client_fd, &read_fds)) {
				if (process_client(client_fd) == -1) {
					close(client_fd);
					FD_CLR(client_fd, &master_fds);
					mark_unused(fds, client_fd, sizeof(fds) / sizeof(fds[0]));
				}
			}
		}

	}

	close(server_fd);

	return 0;
}
