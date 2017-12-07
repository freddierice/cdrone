#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <spdlog/spdlog.h>

#include "controller/Watchdog.h"
#include "misc/Config.h"
#include "misc/exception.h"
#include "program/cdrone.h"
#include "program/test.h"
	
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
void setup_ssl() {
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}
void setup_watchdog() {
	Watchdog::initialize(10);
}

// main program
int main(int argc, const char *argv[]) try {
	int ret = 0;

	setup_loggers();

	Config config("cdrone.conf");
	setup_signals();
	setup_ssl();
	setup_watchdog();

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
		} else if (!strcmp(program, "calibrate")) {
		
		} else if (!strcmp(program, "ssl")) {
			test_ssl(config);
		} else if (!strcmp(program, "camera")) {
			test_camera(config);
		} else if (!strcmp(program, "skyline")) {
			test_skyline(config);
		} else {
			spdlog::get("console")->error("{} is not a valid program", program);
			ret = 1;
		}
	} else {

		// the main event
		spdlog::get("console")->info("starting cdrone");
		cdrone(config);
	}

	spdlog::get("console")->info("joining with Watchdog lib");
	Watchdog::destroy();

	spdlog::get("console")->info("closing all loggers");
	spdlog::drop_all();

	return ret;
} catch (ConfigException& ex) {
	std::cerr << "could not parse config file:" << std::endl << ex.what() <<
		std::endl;
	return 1;
} catch (CDroneException& ex) {
	std::cerr << "unhandled exception: " << std::endl << ex.what() << std::endl;
	return 1;
}
