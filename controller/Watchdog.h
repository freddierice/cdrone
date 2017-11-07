#ifndef __WATCHDOG__
#define __WATCHDOG__
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <spdlog/spdlog.h>

// Watchdog shuts down the program when too much time has passed.
class Watchdog {
public:
	Watchdog(time_t delay_sec, useconds_t delay_ms);
	
	// ok resets the watchdog timer.
	void ok();

	// start starts the watchdog timer.
	void start();

	// stop stops the watchdog timer.
	void stop();
private:
	// TODO: watchdog should register callbacks to reset hardware.
	static void kill(int) {
		spdlog::get("console")->error("watchdog killed program");
		std::exit(1);
	}
	
	// no copying
	Watchdog( const Watchdog& );
	Watchdog& operator=( const Watchdog& );

	time_t m_delay_sec;
	useconds_t m_delay_usec;
};

#endif /* __WATCHDOG__ */
