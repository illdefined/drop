#include <syslog.h>

#include "event.h"
#include "sctp.h"

/**
 * \brief SCTP socket descriptor
 */
int sctp;

/**
 * \brief Thread-local SCTP socket read watcher
 */
__thread struct ev_io sctp_rwatch;

/**
 * \brief Thread-local SCTP socket write watcher
 */
__thread struct ev_io sctp_wwatch;

/**
 * \brief SCTP socket read event handler
 *
 * \param loop event loop
 * \param watcher I/O watcher
 * \param revents event mask
 */
void sctp_revent(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	if (revents & EV_ERROR) {
		syslog(LOG_ERR, "An unspecified error occured in libev");
		exit(EXIT_FAILURE);
	}
	else for (unsigned iter = 0; iter < EVENT_MAX; ++iter) {
		/* Read messages from socket */
	}
}

/**
 * \brief SCTP socket write event handler
 *
 * \param loop event loop
 * \param watcher I/O watcher
 * \param revents event mask
 */
void sctp_wevent(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	if (revents & EV_ERROR) {
		syslog(LOG_ERR, "An unspecified error occured in libev");
		exit(EXIT_FAILURE);
	}
	else for (unsigned iter = 0; iter < EVENT_MAX; ++iter) {
		/* Write messages to socket */
	}
}
