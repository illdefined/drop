#include <syslog.h>

#include "event.h"
#include "unix.h"

/**
 * \brief Unix socket descriptor
 */
int unix;

/**
 * \brief Thread-local Unix socket watcher
 */
__thread struct ev_io unix_watch;

/**
 * \brief Unix socket event handler
 *
 * \param loop event loop
 * \param watcher I/O watcher
 * \param revents event mask
 */
void unix_event(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	if (revents & EV_ERROR) {
		syslog(LOG_ERR, "An unspecified error occured in libev");
		exit(EXIT_FAILURE);
	}
	else for (unsigned iter = 0; iter < EVENT_MAX; ++iter) {
		/* Read messages from socket */
	}
}
