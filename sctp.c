#include <errno.h>
#include <syslog.h>

#include "event.h"
#include "message.h"
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
 * \brief Thread-local message buffer
 */
static __thread struct message msgbuf[MESSAGE_MAX];
static __thread struct sctp_sndrcvinfo sinfo;

/**
 * \brief SCTP socket read event handler
 *
 * \param loop event loop
 * \param watcher I/O watcher
 * \param revents event mask
 */
void sctp_revent(struct ev_loop *loop, struct ev_io *watcher, int revents) {
	if (unlikely(revents & EV_ERROR)) {
		syslog(LOG_ERR, "An unspecified error occured in libev");
		exit(EXIT_FAILURE);
	}
	else for (unsigned iter = 0; iter < EVENT_MAX; ++iter) {
		/* Read messages from socket */
		struct sockaddr_in6 from;
		socklen_t len = sizeof from;
		int flags;

		int ret = sctp_recvmsg(sctp, msgbuf, sizeof msgbuf,
			(struct sockaddr *) &from, &len, &sinfo, &flags);
		if (unlikely(ret < 0)) {
			if (unlikely(errno != EAGAIN && errno != EWOULDBLOCK)) {
				syslog(LOG_ERR, "Failed to read from SCTP socket: %m");
				exit(EXIT_FAILURE);
			}
		}
		else for (unsigned mesg = 0; mesg < ret / sizeof (struct message); ++mesg) {
			/* Process message */
		}
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
