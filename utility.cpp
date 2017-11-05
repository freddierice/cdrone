#include "utility.h"
#include <signal.h>

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
