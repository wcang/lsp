#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h> /* this is needed for struct sockaddr_in (IPv4 address) */
#include <unistd.h>	/* close syscall */
#include <arpa/inet.h>	/* for inet_ntop */



static int connect_server(const char * caddr, uint16_t port, const char * msg)
{
	int sockfd;
	int len;
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

	/* send message to server @ address:port */
	printf("Message length: %lu\n", strlen(msg) + 1);
	len = sendto(sockfd, msg, strlen(msg) + 1, 0, 
				(struct sockaddr *) &addr, sizeof(addr));
	printf("Sent %d bytes\n", len);
	return sockfd;
}


int main(int argc, char * argv[])
{
	int sock_fd;
	unsigned long port;
	char * caddr, * endptr;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s address port_number message\n", argv[0]);
		exit(1);
	}

	caddr = argv[1];
	port = strtoul(argv[2], &endptr, 10);

	if (endptr[0] != 0 || port > UINT16_MAX) {
		fprintf(stderr, "Invalid argument %s for port number\n", argv[2]);
		exit(1);
	}

	sock_fd = connect_server(caddr, port, argv[3]);
	close(sock_fd);

	return 0;
}
