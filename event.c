#include <pthread.h>
#include <ev.h>

#include <errno.h>
#include <stdlib.h>
#include <syslog.h>

#include "predict.h"

#include "icmp.h"
#include "sctp.h"
#include "unix.h"

/**
 * \brief Thread-local event loop
 */
__thread struct ev_loop *loop;

static pthread_once_t event_once = PTHREAD_ONCE_INIT;

/**
 * \brief System error callback
 *
 * \param message error message
 */
static void event_error(const char *restrict message) {
	syslog(LOG_ERR, "%s: %m", message);
	exit(EXIT_FAILURE);
}

/**
 * \brief Global libev initialisation routine
 */
static void event_once_init() {
	ev_set_syserr_cb(event_error);
}

/**
 * \brief Event initialisation routine
 *
 * \param arg unused
 */
void *event_init(void *arg) {
	errno = pthread_once(&event_once, event_once_init);
	if (unlikely(errno)) {
		syslog(LOG_ERR, "Event initialisation failed: %m");
		exit(EXIT_FAILURE);
	}

	loop = ev_loop_new(EVFLAG_AUTO);
	if (unlikely(!loop)) {
		syslog(LOG_ERR, "Could not create event loop");
		exit(EXIT_FAILURE);
	}

	/* Initialise event watchers */
	ev_io_init(&icmp_watch, icmp_event, icmp, EV_READ);
	ev_io_start(loop, &icmp_watch);

	ev_io_init(&sctp_rwatch, sctp_revent, sctp, EV_READ);
	ev_io_init(&sctp_wwatch, sctp_wevent, sctp, EV_WRITE);
	ev_io_start(loop, &sctp_rwatch);

	ev_io_init(&unix_watch, unix_event, unix, EV_READ);
	ev_io_start(loop, &unix_watch);

	/* Enter event loop */
	ev_loop(loop, 0);
	return (void *) 0;
}
