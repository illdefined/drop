#include <pthread.h>

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "event.h"
#include "predict.h"

#include "icmp.h"
#include "sctp.h"
#include "unix.h"

/**
 * \brief Command line argument description
 */
static const char usage[] =
	"Usage: %s [options]\n"
	"\n"
	"  -c CTRL     Control socket\n"
	"  -h          Issue help\n"
	"  -l ADDR     Listen address\n"
	"  -n          Do not fork\n"
	"  -p FILE     PID file\n"
	"  -t NUM      Number of threads\n"
	"  -u USER     Drop privileges\n";

/**
 * \brief Parent process signal handler
 *
 * \param signum signal number
 */
static void parsig(int signum) {
	switch (signum) {
	case SIGALRM: exit(EXIT_FAILURE);
	case SIGCHLD: exit(EXIT_FAILURE);
	case SIGUSR1: exit(EXIT_SUCCESS);
	}
}

/**
 * \brief Main routine
 *
 * \param argc number of arguments
 * \param argv argument vector
 *
 * \return exit status
 */
int main(int argc, char *argv[]) {
	bool daemon = true;
	unsigned long threads = 4;
	const char *ctrl = "/var/run/drop/control";
	const char *lstr = "::";
	const char *pidpath = (char *) 0;
	const char *user = (char *) 0;
	FILE *pidfile = (FILE *) 0;

	/* Parse command line arguments */
	int opt;
	while ((opt = getopt(argc, argv, "c:hl:np:t:u:")) >= 0) {
		switch (opt) {
		case 'c':
			ctrl = optarg;
			break;

		case 'h':
			printf(usage, argv[0]);
			return EXIT_SUCCESS;

		case 'l':
			lstr = optarg;
			break;

		case 'n':
			daemon = false;
			break;

		case 'p':
			pidpath = optarg;
			break;

		case 't':
			threads = strtoul(optarg, (char **) 0, 0);
			break;

		case 'u':
			user = optarg;
			break;

		default:
			printf(usage, argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (unlikely(threads < 1))
		fputs("At least one thread will be used\n", stderr);

	/* Initialise sockets */
	struct in6_addr laddr;
	if (unlikely(inet_pton(AF_INET6, lstr, &laddr) != 1)) {
		fputs("Invalid listen address\n", stderr);
		return EXIT_FAILURE;
	}

	unix = unix_init(ctrl);
	icmp = icmp_init(&laddr);
	sctp = sctp_init(&laddr);

	/* Open PID file */
	if (pidpath) {
		int fd = open(pidpath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
		if (fd < 0) {
			perror("Cannot open process ID file");
			return EXIT_FAILURE;
		}

		pidfile = fdopen(fd, "w");
		if (!pidfile) {
			perror("Cannot associate stream with file descriptor");
			return EXIT_FAILURE;
		}
	}

	/* Drop privileges */
	if (user) {
		struct passwd *passwd = getpwnam(user);
		if (!passwd) {
			perror("Unable to get password file entry");
			return EXIT_FAILURE;
		}

		if (setgid(passwd->pw_gid)) {
			perror("Failed to set group ID");
			return EXIT_FAILURE;
		}

		if (setuid(passwd->pw_uid)) {
			perror("Failed to set user ID");
			return EXIT_FAILURE;
		}
	}

	/* Open system log */
	openlog("drop", 0, LOG_USER);

	pid_t pid;
	pid_t parent;

	if (daemon) {
		struct sigaction action = {
			.sa_handler = parsig,
			.sa_mask    = 0,
			.sa_flags   = 0
		};

		sigaction(SIGALRM, &action, (struct sigaction *) 0);
		sigaction(SIGCHLD, &action, (struct sigaction *) 0);
		sigaction(SIGUSR1, &action, (struct sigaction *) 0);

		/* Fork to background */
		pid = fork();
		if (unlikely(pid < 0)) {
			perror("Cannot fork to background");
			return EXIT_FAILURE;
		}

		else if (pid > 0) {
			alarm(10);
			pause();
			return EXIT_FAILURE;
		}

		parent = getppid();

		/* Create new session */
		if (setsid() < 0) {
			syslog(LOG_ERR, "Unable to create new session: %m");
			kill(parent, SIGCHLD);
			return EXIT_FAILURE;
		}

		if (chdir("/")) {
			syslog(LOG_ERR, "Unable to change directory: %m");
			kill(parent, SIGCHLD);
			return EXIT_FAILURE;
		}

		freopen("/dev/null", "r", stdin);
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);
	}

	/* Save process ID */
	if (pidfile) {
		fprintf(pidfile, "%i", getpid());
		fclose(pidfile);
	}

	/* Spawn worker threads */
	for (unsigned long iter = 1; iter < threads; ++threads) {
		pthread_t thread;

		errno = pthread_create(&thread, (pthread_attr_t *) 0, event_init, (void *) 0);
		if (unlikely(errno)) {
			syslog(LOG_ERR, "Failed to spawn thread: %m");
			if (daemon)
				kill(parent, SIGCHLD);
			return EXIT_FAILURE;
		}
	}

	/* Tell parent that we are okay */
	if (daemon)
		kill(parent, SIGUSR1);

	event_init((void *) 0);
}
