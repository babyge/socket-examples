
/*
 * echo_client.c
 *
 * Datagram (UDP) echo client for use with the echo server.
 *
 *   (terminal 1)
 *   ./echo_server
 *   Port: 16245
 *
 *   (terminal 2)
 *   ./echo_client localhost 16245
 *   (type in characters and the result will be echoed back)
 *
 * Author:
 *   Jeremiah Mahler <jmmahler@gmail.com>
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 1500

static int quit = 0;
void int_handler() {
	quit = 1;
}

int main(int argc, char* argv[]) {
	struct addrinfo hints, *p;
	int n;
	char *host;
	char *port;
	char buf[MAXLINE];
	int sockfd = 0;
	struct addrinfo *res = NULL;
	struct sigaction int_act;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	host = argv[1];
	port = argv[2];

	memset(&int_act, 0, sizeof(int_act));
	int_act.sa_handler = int_handler;
	/* catch Ctrl-C/Ctrl-D */
	if (-1 == sigaction(SIGINT, &int_act, 0)) {
		perror("int sigaction failed");
		exit(EXIT_FAILURE);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family 	= AF_INET;
	hints.ai_socktype 	= SOCK_DGRAM;

	if ( (n = getaddrinfo(host, port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
		exit(EXIT_FAILURE);
	}

	/* loop through addresses, use first one that works */
	for (p = res; p != NULL; p = p->ai_next) {
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (-1 == sockfd) {
			perror("socket");
			continue;
		}

		break;
	}
	if (NULL == p) {
		fprintf(stderr, "No address found.\n");
		exit(EXIT_FAILURE);
	}

	while (!quit) {

		/* get line from user ... */
		if (!fgets(buf, MAXLINE, stdin))
			break;

		n = strlen(buf);

		/* Send output */
		n = sendto(sockfd, buf, n, 0, p->ai_addr, p->ai_addrlen);
		if (n < 0) {
			perror("sendto");
			exit(EXIT_FAILURE);
		}

		/* Receive response */
		n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, NULL);
		if (n < 0) {
			perror("recv");
			exit(EXIT_FAILURE);
		}

		/* make sure it is terminated */
		buf[n] = '\0';

		fputs(buf, stdout);
	}

	if (sockfd > 0)
		close(sockfd);

	if (res)
		freeaddrinfo(res);

	return EXIT_SUCCESS;
}
