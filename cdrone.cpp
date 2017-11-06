#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <openssl/ssl.h>
#include <spdlog/spdlog.h>

#include "Config.h"
#include "Watchdog.h"
#include "Serial.h"
#include "MultiWii.h"
#include "Infrared.h"
#include "main.h"
#include "utility.h"
	
void do_update(Watchdog &watchdog, Infrared &infrared) {
	
	// start loop
	spdlog::get("console")->info("update loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		infrared.update();
	}
}

void do_io(Watchdog &watchdog, Config &config) {

	spdlog::get("console")->info("initializing io server");

	
	spdlog::get("console")->info("io loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		usleep(100000);
	}
}

void do_analysis(Watchdog &watchdog, Config &config) {

	spdlog::get("console")->info("analysis loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		usleep(100000);
	}
}

void do_controller(Watchdog &watchdog, Config &config) {
	
	spdlog::get("console")->info("controller loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		usleep(100000);
	}
}

void cdrone(Config &config) {

	// initialize the watchdogs
	spdlog::get("console")->info("initializing watchdogs");
	Watchdog updateWatchdog(config.updateWatchdog(), 0);
	Watchdog ioWatchdog(config.ioWatchdog(), 0);
	Watchdog analysisWatchdog(config.analysisWatchdog(), 0);
	Watchdog controllerWatchdog(config.controllerWatchdog(), 0);
	
	// initialize the hardware
	spdlog::get("console")->info("initializing infrared");
	Infrared infrared(config.infraredAlpha(), config.infraredB(), 
			config.infraredK());

	// create threads
	spdlog::get("console")->info("creating threads");
	pthread_block(SIGINT);
	std::thread update_thr(do_update, std::ref(updateWatchdog), 
			std::ref(infrared));
	std::thread analysis_thr(do_analysis, std::ref(analysisWatchdog), 
			std::ref(config));
	std::thread io_thr(do_io, std::ref(ioWatchdog), std::ref(config));
	std::thread controller_thr(do_controller, std::ref(controllerWatchdog), 
			std::ref(config));
	pthread_unblock(SIGINT);
	
	// join with threads
	update_thr.join(); 
	spdlog::get("console")->info("update thread joined");
	io_thr.join(); 
	spdlog::get("console")->info("io thread joined");
	analysis_thr.join(); 
	spdlog::get("console")->info("analysis thread joined");
	controller_thr.join();
	spdlog::get("console")->info("controller thread joined");
}

