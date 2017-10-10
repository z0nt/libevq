/*-
 * Copyright (c) 2016 Andrey Zonov
 * All rights reserved.
*/

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define setnonblock(s)	fcntl((s), F_SETFL, fcntl((s), F_GETFL) | O_NONBLOCK)

int
sock_listen(const char *ip, unsigned short port)
{
	int sockfd, reuse = 1;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_aton(ip, &addr.sin_addr) == 0) {
		errno = EINVAL;
		warn("inet_aton");
		return (-1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		warn("socket");
		return (-1);
	}

	if (setnonblock(sockfd) == -1) {
		warn("fcntl(O_NONBLOCK)");
		close(sockfd);
		return (-1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
		warn("setsockopt(SO_REUSEADDR)");
		close(sockfd);
		return (-1);
	}

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		warn("bind");
		close(sockfd);
		return (-1);
	}

	if (listen(sockfd, -1) == -1) {
		warn("listen");
		close(sockfd);
		return (-1);
	}

	return (sockfd);
}

int
sock_accept(int sockfd)
{
	int newfd;

	newfd = accept(sockfd, NULL, NULL);
	if (newfd == -1) {
		warn("accept");
		return (-1);
	}

	if (setnonblock(newfd) == -1) {
		warn("fcntl(O_NONBLOCK)");
		close(newfd);
		return (-1);
	}

	return (newfd);
}
