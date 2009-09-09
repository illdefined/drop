.POSIX:

CC = clang
LDFLAGS = -lev -lsctp

drop: drop.c event.c icmp.c sctp.c unix.c
	$(CC) -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -pthread -std=c99 -Wall $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f drop
