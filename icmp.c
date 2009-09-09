#include <syslog.h>

#include "event.h"
#include "icmp.h"

/**
 * \brief ICMP socket descriptor
 */
int icmp;

/**
 * \brief Thread-local ICMP socket watcher
 */
__thread struct ev_io icmp_watch;

/**
 * \brief ICMP socket event handler
 *
 * \param loop event loop
 * \param watcher I/O watcher
 * \param revents event mask
 */
void icmp_event(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	if (revents & EV_ERROR) {
		syslog(LOG_ERR, "An unspecified error occured in libev");
		exit(EXIT_FAILURE);
	}
	else for (unsigned iter = 0; iter < EVENT_MAX; ++iter) {
		/* Read messages from socket */
	}
}
