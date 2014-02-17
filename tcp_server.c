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


void process_client(int client_fd)
{
	int len;
	uint8_t data_len = 0;
	uint16_t remainder = 256;
	char data[256];
	char * ptr;
	ptr = data;

	while (!term && remainder) {
		len = recv(client_fd, ptr, remainder, 0);

		if (len == -1) {
			fprintf(stderr, "reception of TCP data failed: %m\n");
			return;
		}
		else if (len == 0) {
			fprintf(stderr, "connection closed while %u remaining bytes is expected\n", remainder);
			return;
		}

		if (len > remainder) {
			fprintf(stderr, "more data (%d bytes) is sent than what is expected.\n", 
					len - remainder);
			return;
		}

		if (remainder == 256) {
			remainder = data_len = data[0];
			ptr++;
			--len;
		}

		remainder -= len;
		ptr += len;

		if (remainder == 0) {
			data[255] = 0; /* just to ensure, non-null terminated string won't crash the system */
			printf("%s\n", &data[1]);
		}
	}
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


int main(int argc, char * argv[])
{
	int server_fd, client_fd;
	unsigned long port;
	struct sockaddr_in peer_addr;
	char * caddr, * endptr;
	char cpeer_addr[20];
	socklen_t addr_len;

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

	while (!term) {
		memset(&peer_addr, 0, sizeof(peer_addr));
		addr_len = sizeof(peer_addr);
		client_fd = accept(server_fd, (struct sockaddr *) &peer_addr, &addr_len);

		if (client_fd == -1) {
			fprintf(stderr, "Failed to accept TCP connection: %m\n");
			continue;
		}

		inet_ntop(AF_INET, &peer_addr.sin_addr, cpeer_addr, sizeof(cpeer_addr));
	       	printf("Receive connection from %s:%u\n", cpeer_addr, ntohs(peer_addr.sin_port));
		process_client(client_fd);
		close(client_fd);
	}

	close(server_fd);

	return 0;
}
