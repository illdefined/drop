#pragma once
#ifndef UNIX_H
#define UNIX_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "predict.h"

extern int unix;

static inline int unix_init(const char *restrict path) {
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
	};

	if (unlikely(strlen(path) >= sizeof(addr.sun_path))) {
		fputs("Socket path name too long\n", stderr);
		exit(EXIT_FAILURE);
	}

	strcpy(addr.sun_path, path);

	int fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (unlikely(fd < 0)) {
		perror("Cannot create socket");
		exit(EXIT_FAILURE);
	}

	/* Bind socket */
	if (unlikely(bind(fd, (struct sockaddr *) &addr, SUN_LEN(&addr)))) {
		perror("Cannot bind to address");
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
