#include "main.h"
#include "controller/FlightController.h"
#include "controller/IOController.h"
#include "controller/Watchdog.h"
#include "hardware/Camera.h"
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
	// spdlog::get("console")->info("update loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		infrared.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}
	watchdog.stop();
}

void do_io(Watchdog &watchdog, Config &config, 
		FlightController &flightController, 
		std::shared_ptr<Observations> obs, Camera &camera) {

	// auto console = spdlog::get("console");
	
	// console->info("initializing io server");
	while (!shutdown) {
		// wait for someone to connect.
		watchdog.stop();
		IOController ioController(config);
		while (!shutdown && !ioController.connected()) {
			try {
				ioController.accept();
			} catch (ServerException &ex) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}

		// console->info("io loop started");
		watchdog.start();
		while (!shutdown) {
			proto::Update update;
			try {
				watchdog.stop();
				ioController.getMessage(update);
			} catch (IOControllerException &ex) {
				spdlog::get("console")->info("connection closed: {}", ex.what());
				flightController.disarm();
				break;
			}
			watchdog.start();
			
			// check the update mode
			switch (update.mode()) {
				case proto::UpdateMode::NO_MODE_CHANGE:
					break;
				case proto::UpdateMode::ARM:
					// console->info("io ARM");
					flightController.arm();
					break;
				case proto::UpdateMode::DISARM:
					// console->info("io DISARM");
					flightController.disarm();
					break;
				case proto::UpdateMode::TAKEOFF:
					flightController.takeoff();
					break;
				default:
					// SPDLOG_DEBUG(console, "unknown message");
					break;
			}

			// check for any commands.
			switch (update.cmd()) {
				case proto::UpdateCommand::NO_COMMAND:
					break;
				case proto::UpdateCommand::RESET_POSITION:
					// console->info("io RESET_POSITION");
					camera.resetPosition();
					break;
				default:
					camera.resetPosition();
					// console->warn("got unknown UpdateCommand");
			}
			
			// check the velocity mode
			if (update.has_velocity()) {
				auto velocity = update.velocity();
				obs->ioVelocityRoll = velocity.roll();
				obs->ioVelocityPitch = velocity.pitch();
				obs->ioVelocityYaw = velocity.yaw();
				obs->ioVelocityThrottle = velocity.throttle();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	watchdog.stop();
}

void do_analysis(Watchdog &watchdog, Config &config) {

	// spdlog::get("console")->info("analysis loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	watchdog.stop();
}

void do_controller(Watchdog &watchdog, Config &config, 
		FlightController &flightController) {
	
	// spdlog::get("console")->info("controller loop started");
		
	watchdog.start();
	flightController.calibrate();
	while (!shutdown) {
		watchdog.ok();
		flightController.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	watchdog.stop();
}

void cdrone(Config &config) {

	// initialize the watchdogs
	// spdlog::get("console")->info("initializing watchdogs");
	Watchdog updateWatchdog(config.updateWatchdog(), "update");
	Watchdog ioWatchdog(config.ioWatchdog(), "io");
	Watchdog analysisWatchdog(config.analysisWatchdog(), "analysis");
	Watchdog controllerWatchdog(config.controllerWatchdog(), "controller");

	// initialize the observations
	std::shared_ptr<Observations> obs = std::make_shared<Observations>();
	
	// initialize the hardware
	// spdlog::get("console")->info("initializing infrared");
	FlightController flightController(config, obs);
	Infrared infrared(config, obs);
	Camera camera(config, obs);

	// start camera
	camera.start();

	// create threads
	// spdlog::get("console")->info("creating threads");
	pthread_block(SIGINT);
	std::thread update_thr(do_update, std::ref(updateWatchdog), 
			std::ref(infrared));
	std::thread analysis_thr(do_analysis, std::ref(analysisWatchdog), 
			std::ref(config));
	std::thread io_thr(do_io, std::ref(ioWatchdog), std::ref(config),
			std::ref(flightController), obs, std::ref(camera));
	std::thread controller_thr(do_controller, std::ref(controllerWatchdog), 
			std::ref(config), std::ref(flightController));
	pthread_unblock(SIGINT);
	
	// join with threads
	update_thr.join(); 
	// spdlog::get("console")->info("update thread joined");
	io_thr.join();
	// spdlog::get("console")->info("io thread joined");
	analysis_thr.join(); 
	// spdlog::get("console")->info("analysis thread joined");
	controller_thr.join();
	// spdlog::get("console")->info("controller thread joined");
	
	// stop camera
	camera.stop();
}

