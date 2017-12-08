#include <atomic>
#include <iostream>
#include <thread>

#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "controller/FlightController.h"
#include "controller/IOController.h"
#include "controller/Watchdog.h"
#include "hardware/Camera.h"
#include "hardware/Infrared.h"
#include "main.h"
#include "misc/Config.h"
#include "misc/logging.h"
#include "misc/utility.h"
#include "wire/MultiWii.h"
#include "wire/Serial.h"


void do_update(Watchdog &watchdog, Infrared &infrared) {
	
	console->info("update loop started");
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

	console->info("initializing io server");
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

		console->info("io loop started");
		watchdog.start();
		while (!shutdown) {
			proto::Update update;
			try {
				ioController.getMessage(update);
				watchdog.ok();
			} catch (IOControllerException &ex) {
				console->info("connection closed: {}", ex.what());
				flightController.disarm();
				break;
			}
			
			// check the update mode
			switch (update.mode()) {
				case proto::UpdateMode::NO_MODE_CHANGE:
					break;
				case proto::UpdateMode::ARM:
					console->info("io ARM");
					flightController.arm();
					break;
				case proto::UpdateMode::DISARM:
					console->info("io DISARM");
					flightController.disarm();
					break;
				case proto::UpdateMode::TAKEOFF:
					flightController.takeoff();
					break;
				default:
					console->warn("received unknown mode {}", update.mode());
					break;
			}

			// check for any commands.
			switch (update.cmd()) {
				case proto::UpdateCommand::NO_COMMAND:
					break;
				case proto::UpdateCommand::RESET_POSITION:
					console->info("io RESET_POSITION");
					camera.resetPosition();
					break;
				default:
					console->warn("got unknown UpdateCommand");
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

/*
void do_analysis(Watchdog &watchdog, Config &config) {

	console->info("analysis loop started");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	watchdog.stop();
}
*/

void do_controller(Watchdog &watchdog, Config &config, 
		FlightController &flightController) {
	
	console->info("controller loop started");
		
	watchdog.start();
	flightController.calibrate();
	while (!shutdown) {
		watchdog.ok();
		flightController.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	watchdog.stop();
}

void cdrone(Config &config) {

	// initialize the watchdogs
	console->info("initializing watchdogs");
	Watchdog updateWatchdog(config.updateWatchdog(), "update");
	Watchdog ioWatchdog(config.ioWatchdog(), "io");
	// Watchdog analysisWatchdog(config.analysisWatchdog(), "analysis");
	Watchdog controllerWatchdog(config.controllerWatchdog(), "controller");

	// initialize the observations
	std::shared_ptr<Observations> obs = std::make_shared<Observations>();
	
	// initialize the hardware
	console->info("initializing infrared");
	FlightController flightController(config, obs);
	Infrared infrared(config, obs);
	Camera camera(config, obs);

	// start camera
	camera.start();

	// create threads
	console->info("creating threads");
	pthread_block(SIGINT);
	std::thread update_thr(do_update, std::ref(updateWatchdog), 
			std::ref(infrared));
	// std::thread analysis_thr(do_analysis, std::ref(analysisWatchdog), 
	//		std::ref(config));
	std::thread io_thr(do_io, std::ref(ioWatchdog), std::ref(config),
			std::ref(flightController), obs, std::ref(camera));
	std::thread controller_thr(do_controller, std::ref(controllerWatchdog), 
			std::ref(config), std::ref(flightController));
	pthread_unblock(SIGINT);
	
	// join with threads
	update_thr.join(); 
	console->info("update thread joined");
	io_thr.join();
	console->info("io thread joined");
	// analysis_thr.join(); 
	// console->info("analysis thread joined");
	controller_thr.join();
	console->info("controller thread joined");
	
	// stop camera
	camera.stop();
}

