#include "utility.h"
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
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
	return (double)ts.tv_sec + ((double)ts.tv_nsec)/1000000000;
}

