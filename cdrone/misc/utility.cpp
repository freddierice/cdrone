#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "misc/utility.h"

#include <spdlog/spdlog.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


void pthread_block(int sig) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	pthread_sigmask(SIG_BLOCK, &ss, 0);
}

void pthread_unblock(int sig) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	pthread_sigmask(SIG_UNBLOCK, &ss, 0);
}

double get_time() {
	struct timespec ts; 
	::clock_gettime(CLOCK_REALTIME, &ts);
	return (double)ts.tv_sec + ((double)ts.tv_nsec)/1000000000.0;
}

int writeFull(int fd, const char *buffer, int len) {
	int ret, total = 0;
	do {
again:
		if ((ret = write(fd, buffer+total, len-total)) < 0) {
			if (errno == EINTR || errno == EAGAIN)
				goto again;
			return ret;
		}
		total += ret;
	} while(len != total);

	return total;
}
