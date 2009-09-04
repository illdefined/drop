#pragma once
#ifndef SCTP_H
#define SCTP_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include "predict.h"

#define SCTP_PORT 235
#define SCTP_BACKLOG 16

extern int sctp;

static inline int sctp_init(const struct in6_addr *restrict laddr) {
	struct sockaddr_in6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(SCTP_PORT),
		.sin6_flowinfo = 0,
		.sin6_scope_id = 0
	};

	addr.sin6_addr = *laddr;

	int fd = socket(PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
	if (unlikely(fd < 0)) {
		perror("Cannot create socket");
		exit(EXIT_FAILURE);
	}

	/* Bind socket */
	if (unlikely(bind(fd, (struct sockaddr *) &addr, sizeof addr))) {
		perror("Cannot bind to address");
		exit(EXIT_FAILURE);
	}

	if (unlikely(listen(sctp, SCTP_BACKLOG))) {
		perror("Unable to listen on socket");
		exit(EXIT_FAILURE);
	}

	/* Mark socket non-blocking */
	if (unlikely(fcntl(fd, F_SETFL, O_NONBLOCK))) {
		perror("Unable to mark socket non-blocking");
		exit(EXIT_FAILURE);
	}

	return fd;
}

#endif
