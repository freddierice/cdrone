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

void do_recv(Watchdog &watchdog, Config &config, 
		FlightController &flightController,
		std::shared_ptr<IO> io,
		std::shared_ptr<Observations> obs, Camera &camera) {
	proto::Update update;

	// try to get the first message before starting the watchdog.
	try {
		while(!io->recvMessage(update) && !shutdown)
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
	} catch (IOException &ex) {
		console->info("error recieving: {}", ex.what());
		return;
	}
	
	console->info("recv loop started");
	watchdog.start();
	while (!shutdown) {
		// check the update mode
		switch (update.mode()) {
			case proto::NO_MODE:
				break;
			case proto::ARM:
				console->info("io ARM");
				flightController.arm();
				break;
			case proto::DISARM:
				console->info("io DISARM");
				flightController.disarm();
				obs->resetIO();
				break;
			case proto::TAKEOFF:
				flightController.takeoff();
				break;
			case proto::RAW:
				flightController.rawControl();
				break;
			case proto::VELOCITY:
				flightController.velocityControl();
				break;
			case proto::POSITION:
				flightController.positionControl();
				break;
			default:
				console->warn("received unknown mode {}", update.mode());
				break;
		}

		// check for any commands.
		switch (update.command()) {
			case proto::NO_COMMAND:
				break;
			case proto::RESET_POSITION:
				console->info("io RESET_POSITION");
				camera.resetPosition();
				break;
			case proto::DISCONNECT:
				flightController.disarm();
				watchdog.stop();
				return;
			default:
				console->warn("got unknown UpdateCommand");
		}
		
		// check the raw mode
		if (update.has_raw()) {
			auto raw = update.raw();
			obs->ioRawRoll = raw.roll();
			obs->ioRawPitch = raw.pitch();
			obs->ioRawYaw = raw.yaw();
			obs->ioRawThrottle = raw.throttle();
		}

		// check the velocity mode
		if (update.has_velocity()) {
			auto velocity = update.velocity();
			obs->ioVelocityX = velocity.x();
			obs->ioVelocityY = velocity.y();
			obs->ioPositionZ = velocity.z();
		}

		// check the position mode
		if (update.has_position()) {
			auto position = update.position();
			obs->ioPositionX = position.x();
			obs->ioPositionY = position.y();
			obs->ioPositionZ = position.z();
		}
		
		// get another message
		try {
			watchdog.ok();
			update.Clear();
			while(!io->recvMessage(update) && !shutdown)
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			watchdog.ok();
		} catch (IOException &ex) {
			console->info("error recieving: {}", ex.what());
			flightController.disarm();
			watchdog.stop();
			return;
		}
	}
	watchdog.stop();
}

void do_send(std::shared_ptr<IO> io, std::shared_ptr<Observations> obs,
		FlightController &flightController) {
	proto::Observations protoObs;

	console->info("send loop started");
	while (!shutdown) {
		protoObs.Clear();
		protoObs.set_battery(obs->skylineBattery);
		auto mode = flightController.getMode();
		switch (mode) {
			case Disarmed:
			case Disarming:
				protoObs.set_mode(proto::DISARM);
				break;
			case Arming:
			case Armed:
				protoObs.set_mode(proto::ARM);
				break;
			case TakeOff:
				protoObs.set_mode(proto::TAKEOFF);
				break;
			case TouchDown:
				protoObs.set_mode(proto::TOUCHDOWN);
				break;
			case RawControl:
				protoObs.set_mode(proto::RAW);
				break;
			case VelocityControl:
				protoObs.set_mode(proto::VELOCITY);
				break;
			case PositionControl:
				protoObs.set_mode(proto::POSITION);
				break;
			case Calibrating:
				protoObs.set_mode(proto::CALIBRATE);
			default:
				// do nothing
				break;
		}
		try {
			io->sendMessage(protoObs);
		} catch (IOException &ex) {
			console->warn("could not send message: {}", ex.what());
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void do_serve(Watchdog &watchdog, Config &config,
		IOController &ioController,
		FlightController &flightController,
		std::shared_ptr<Observations> obs, Camera &camera) {
	while (!shutdown) {
		try {
			std::shared_ptr<IO> io = ioController.accept();
			console->info("server accepted");
			std::thread recv_thr(do_recv, std::ref(watchdog), std::ref(config),
					std::ref(flightController), io, obs,
					std::ref(camera));
			std::thread send_thr(do_send, io, obs,
					std::ref(flightController));
			recv_thr.join();
			console->info("recv joined");
			send_thr.join();
			console->info("send joined");
		} catch (IOControllerException &ex) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
}

void do_controller(Watchdog &watchdog, Config &config, 
		FlightController &flightController) {
	
	console->info("controller loop started");
		
	watchdog.start();
	flightController.calibrate();
	while (!shutdown) {
		watchdog.ok();
		flightController.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
	watchdog.stop();
}

void cdrone(Config &config) {

	// initialize the watchdogs
	console->info("initializing watchdogs");
	Watchdog updateWatchdog(config.updateWatchdog(), "update");
	Watchdog ioWatchdog(config.ioWatchdog(), "io");
	Watchdog controllerWatchdog(config.controllerWatchdog(), "controller");

	// initialize the observations
	std::shared_ptr<Observations> obs = std::make_shared<Observations>();
	
	// initialize the hardware
	console->info("initializing Infrared");
	Infrared infrared(config, obs);

	console->info("initializing FlightController");
	FlightController flightController(config, obs);

	console->info("initializing IOController");
	IOController ioController(config);

	console->info("initializing Camera");
	Camera camera(config, obs);

	// start camera
	console->info("starting Camera");
	camera.start();

	// create threads
	console->info("creating threads");
	pthread_block(SIGINT);
	std::thread update_thr(do_update, std::ref(updateWatchdog), 
			std::ref(infrared));
	std::thread controller_thr(do_controller, std::ref(controllerWatchdog), 
			std::ref(config), std::ref(flightController));
	std::thread serve_thr(do_serve, std::ref(ioWatchdog), std::ref(config),
			std::ref(ioController), std::ref(flightController), obs,
			std::ref(camera));
	pthread_unblock(SIGINT);
	
	// join with threads
	update_thr.join(); 
	console->info("update thread joined");
	serve_thr.join();
	console->info("io thread joined");
	controller_thr.join();
	console->info("controller thread joined");
	
	// stop camera
	camera.stop();
}

