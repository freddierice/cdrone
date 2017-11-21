#ifndef __WATCHDOG__
#define __WATCHDOG__
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <mutex>

#include <spdlog/spdlog.h>


// Watchdog shuts down the program when too much time has passed.
class Watchdog {
public:
	// initialize the Watchdog library to avoid registering threads.
	static void initialize(int n);

	// destroy the Watchdog library to release resources.
	static void destroy();
	
	// create a watchdog with a certain timeout. Throws an exception if 
	// too many watchdogs are created.
	Watchdog(std::chrono::milliseconds timeout);
	
	// ok resets the watchdog timer.
	void ok();

	// start starts the watchdog timer.
	void start();

	// stop stops the watchdog timer.
	void stop();
private:
	/* 
	static void kill(int) {
		spdlog::get("console")->error("watchdog killed program");
		std::exit(1);
	}
	*/

	// no construction without registration
	Watchdog();
	
	// no copying
	Watchdog( const Watchdog& );
	Watchdog& operator=( const Watchdog& );

	// static thread for checking wthe code
	static void check_thread();

	static uint64_t m_nWatchdogs;
	static std::atomic<uint64_t> m_nextThreadID;
	static std::atomic<uint64_t>* m_counts;
	static uint64_t* m_maxes;
	static std::thread m_thread;
	static std::atomic<bool> m_shutdown;
	static const std::chrono::milliseconds EPOCH;
	
	uint64_t m_id;
	std::chrono::milliseconds m_duration;
};

#endif /* __WATCHDOG__ */
