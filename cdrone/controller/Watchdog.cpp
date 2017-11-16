#include <signal.h>
#include <sys/time.h>
#include <spdlog/spdlog.h>
#include "Watchdog.h"

Watchdog::Watchdog(time_t delay_sec, useconds_t delay_usec) : m_delay_sec(delay_sec), m_delay_usec(delay_usec) {
	signal(SIGALRM, Watchdog::kill);
}

void Watchdog::start() {
	this->ok();
}

void Watchdog::ok() {
	struct itimerval t;
	t.it_value.tv_sec  = m_delay_sec;
	t.it_value.tv_usec = m_delay_usec;
	t.it_interval.tv_sec  = 0;
	t.it_interval.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &t, NULL)) {
		spdlog::get("console")->error("watchdog could not set timer");
		std::exit(1);
	}
}

void Watchdog::stop() {
	struct itimerval t;
	t.it_value.tv_sec  = 0;
	t.it_value.tv_usec = 0;
	t.it_interval.tv_sec  = 0;
	t.it_interval.tv_usec = 0;

	if (setitimer(ITIMER_REAL, &t, NULL)) {
		spdlog::get("console")->error("watchdog could not set timer");
		std::exit(1);
	}
}
