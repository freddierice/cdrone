#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <spdlog/spdlog.h>

#include "Config.h"
#include "Watchdog.h"
#include "Serial.h"
#include "MultiWii.h"
#include "Infrared.h"
#include "cdrone.h"
#include "test.h"
#include "utility.h"
	
// shutdown handlers.
std::atomic_bool shutdown;
void shutdown_handler(int sig) {
	shutdown = true;
	spdlog::get("console")->warn("shutting down");
}
void setup_signals() {
	signal(SIGINT, shutdown_handler);
}
void setup_loggers() {
	spdlog::stdout_color_mt("console");
}

// main program
int main(int argc, const char *argv[]) try {
	Config config("cdrone.conf");
	setup_signals();
	setup_loggers();


	// run program for testing
	const char *program = argv[1];
	if (argc >= 2) {
		if (!strcmp(program, "infrared")) {
			spdlog::get("console")->info("starting test_infrared");
			test_infrared(config);
		} else if (!strcmp(program, "multiwii")) {
			spdlog::get("console")->info("starting test_multiwii");
			test_multiwii(config);
		} else if (!strcmp(program, "watchdog")) {
			spdlog::get("console")->info("starting test_watchdog");
			test_watchdog(config);
		} else {
			spdlog::get("console")->error("{} is not a valid program", program);
			return 1;
		}

		return 0;
	}

	// the main event
	spdlog::get("console")->info("starting cdrone");
	cdrone(config);

	// close the loggers
	spdlog::drop_all();

	return 0;
} catch (ConfigException& ex) {
	std::cerr << "could not parse config file" << std::endl;
	return 1;
}

