#include "main.h"
#include "controller/FlightController.h"
#include "controller/IOController.h"
#include "controller/Watchdog.h"
#include "hardware/Infrared.h"
#include "misc/Config.h"
#include "misc/utility.h"
#include "wire/MultiWii.h"
#include "wire/Serial.h"
	
#include <atomic>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <openssl/ssl.h>
#include <spdlog/spdlog.h>

void do_update(Watchdog &watchdog, Infrared &infrared) {
	
	// start loop
	spdlog::get("console")->info("update loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		infrared.update();
		usleep(100);
	}
}

void do_io(Watchdog &watchdog, Config &config, 
		FlightController &flightController) {

	auto console = spdlog::get("console");
	
	console->info("initializing io server");
	while (!shutdown) {
		// wait for someone to connect.
		watchdog.stop();
		IOController ioController(config);

		console->info("io loop started");
		watchdog.start();
		while (!shutdown) {
			io::Update update;
			ioController.getMessage(update);
			watchdog.ok();
			switch (update.mode()) {
				case io::UpdateMode::NO_MODE_CHANGE:
					// SPDLOG_DEBUG(console, "no mode change");
					break;
				case io::UpdateMode::ARM:
					SPDLOG_DEBUG(console, "arming");
					flightController.arm();
					break;
				case io::UpdateMode::DISARM:
					SPDLOG_DEBUG(console, "disarming");
					flightController.disarm();
					break;
				default:
					SPDLOG_DEBUG(console, "unknown message");
					break;
			}
			if (update.has_velocity()) {
				SPDLOG_DEBUG(console, "velocity nyi");
			}
			usleep(100);
		}
	}
}

void do_analysis(Watchdog &watchdog, Config &config) {

	spdlog::get("console")->info("analysis loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		usleep(100);
	}
}

void do_controller(Watchdog &watchdog, Config &config, 
		FlightController &flightController) {
	
	spdlog::get("console")->info("controller loop started");
	while(!shutdown) {
		
		watchdog.start();
		while (!shutdown) {
			watchdog.ok();
			flightController.update();
			usleep(100);
		}
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
	FlightController flightController(config);
	Infrared infrared(config.infraredAlpha(), config.infraredB(), 
			config.infraredK());

	// create threads
	spdlog::get("console")->info("creating threads");
	pthread_block(SIGINT);
	std::thread update_thr(do_update, std::ref(updateWatchdog), 
			std::ref(infrared));
	std::thread analysis_thr(do_analysis, std::ref(analysisWatchdog), 
			std::ref(config));
	std::thread io_thr(do_io, std::ref(ioWatchdog), std::ref(config),
			std::ref(flightController));
	std::thread controller_thr(do_controller, std::ref(controllerWatchdog), 
			std::ref(config), std::ref(flightController));
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

