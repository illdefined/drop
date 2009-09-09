#pragma once
#ifndef ICMP_H
#define ICMP_H

#include <ev.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>

#include "predict.h"

extern int icmp;

extern __thread struct ev_io icmp_watch;

void icmp_event(struct ev_loop *, struct ev_io *, int);

/**
 * \brief ICMP socket initialisation routine
 *
 * \param laddr listen address
 *
 * \return file descriptor
 */
static inline int icmp_init(const struct in6_addr *restrict laddr) {
	struct sockaddr_in6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = 0,
		.sin6_flowinfo = 0,
		.sin6_scope_id = 0
	};

	addr.sin6_addr = *laddr;

	int fd = socket(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (unlikely(fd < 0)) {
		perror("Cannot create socket");
		exit(EXIT_FAILURE);
	}

	/* Filter everything except echo request and replies */
	struct icmp6_filter filter;
	ICMP6_FILTER_SETBLOCKALL(&filter);
	ICMP6_FILTER_SETPASS(ICMP6_ECHO_REQUEST, &filter);
	ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filter);

	if (unlikely(setsockopt(fd, IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof filter))) {
		perror("Unable to set ICMPv6 filters up");
		exit(EXIT_FAILURE);
	}

	/* Bind socket */
	if (unlikely(bind(fd, (struct sockaddr *) &addr, sizeof addr))) {
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
