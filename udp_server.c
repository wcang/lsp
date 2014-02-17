#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h> /* this is needed for struct sockaddr_in (IPv4 address) */
#include <unistd.h>	/* close syscall */
#include <arpa/inet.h>	/* for inet_ntop */
#include <signal.h>	/* sigaction */


volatile sig_atomic_t term = 0;


static int setup_socket(const char * caddr, uint16_t port)
{
	int sockfd;
	struct sockaddr_in addr;

	/* AF_INET: IPv4 protocol
	 * SOCK_DGRAM: UDP protocol
	 */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "Socket creation failed: %s\n", strerror(errno));
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

	return sockfd;
}


void send_msg(int sockfd, char * msg)
{
	uint8_t len;

	if (strlen(msg) > 254) { /* truncate our string because we limit to 255 character msg (including null byte) */
		msg[254] = 0;
	}

	len = strlen(msg) + 1;
	/* FIXME: add checking for send return length */
	send(sockfd, &len, 1, 0);
	send(sockfd, msg, strlen(msg) + 1, 0);
}


static void signal_handler(int signum)
{
	if (signum == SIGTERM) {
		term = 1;
	}
}


static void install_sighandler(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;
	
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		fprintf(stderr, "Unable to install signal handler for SIGTERM: %m\n");
		exit(1);
	}
}



int main(int argc, char * argv[])
{
	int sock_fd;
	unsigned long port;
	char * caddr, * endptr;
	struct sockaddr_in peer_addr;
	char cpeer_addr[20];
	char buffer[255];
	socklen_t addr_len;
	int data_len;

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
	sock_fd = setup_socket(caddr, port);

	while (!term) {
		addr_len = sizeof(peer_addr);
		data_len = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
				(struct sockaddr *) &peer_addr, &addr_len);

		if (data_len == -1) {
			fprintf(stderr, "Error receiving data from UDP socket:%m\n");
			continue;
		}

		if (addr_len > sizeof(peer_addr)) {
			printf("Address provided is greater than storage of IPv4 address\n");
		}
		else {
			inet_ntop(AF_INET, &peer_addr.sin_addr, cpeer_addr, sizeof(cpeer_addr));
			printf("Receive connection from %s:%u\n", cpeer_addr, ntohs(peer_addr.sin_port));
		}

		buffer[sizeof(buffer) - 1] = 0; /* null terminate the string just to be safe */
		printf("%s\n", buffer);
	}
	
	close(sock_fd);
	return 0;
}
