#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "predict.h"

#include "icmp.h"
#include "sctp.h"
#include "unix.h"

static const char usage[] =
	"Usage: %s [options]\n"
	"\n"
	"  -c CTRL     Control socket\n"
	"  -h          Issue help\n"
	"  -l ADDR     Listen address\n"
	"  -n          Do not fork\n"
	"  -p FILE     PID file\n"
	"  -u USER     Drop privileges\n";

static void parsig(int signum) {
	switch (signum) {
	case SIGCHLD: exit(EXIT_FAILURE);
	case SIGUSR1: exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[]) {
	bool daemon = true;
	const char *ctrl = "/var/run/drop/control";
	const char *lstr = "::";
	const char *pidpath = (char *) 0;
	const char *user = (char *) 0;
	FILE *pidfile = (FILE *) 0;

	/* Parse command-line arguments */
	int opt;
	while ((opt = getopt(argc, argv, "c:hl:np:u:")) >= 0) {
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

		case 'u':
			user = optarg;
			break;

		default:
			printf(usage, argv[0]);
			return EXIT_FAILURE;
		}
	}

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

	if (daemon) {
		struct sigaction action = {
			.sa_handler = parsig,
			.sa_mask    = 0,
			.sa_flags   = 0
		};

		sigaction(SIGCHLD, &action, (struct sigaction *) 0);
		sigaction(SIGUSR1, &action, (struct sigaction *) 0);

		/* Fork to background */
		pid_t pid = fork();
		if (unlikely(pid < 0)) {
			perror("Cannot fork to background");
			return EXIT_FAILURE;
		}

		else if (pid > 0) {
			pause();
			return EXIT_FAILURE;
		}

		pid_t parent = getppid();

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

		/* Save process ID */
		if (pidfile) {
			fprintf(pidfile, "%i", getpid());
			fclose(pidfile);
		}

		/* Tell parent that we are okay */
		kill(parent, SIGUSR1);
	}

	else if (pidfile) {
		fprintf(pidfile, "%i", getpid());
		fclose(pidfile);
	}
}
