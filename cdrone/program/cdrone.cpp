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
// #include "hardware/Infrared.h"
#include "main.h"
#include "misc/Config.h"
#include "logging/logging.h"
#include "misc/utility.h"
#include "wire/MultiWii.h"
#include "wire/Serial.h"
#include "position/VRPN.h"


void do_update(Watchdog &watchdog, /*Infrared &infrared,*/
		FlightController &flightController) {
	
	console->info("update loop started");
	watchdog.start();
	while (!global::shutdown) {
		// add watchdog between each update for higher granularity
		watchdog.ok();
		// infrared.update();
		// watchdog.ok();
		flightController.updateRC();
		watchdog.ok();
		flightController.updateController();
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
		while(!io->recvMessage(update) && !global::shutdown) {}
	} catch (IOException &ex) {
		console->info("error recieving: {}", ex.what());
		return;
	}
	
	console->info("recv loop started");
	watchdog.start();
	while (!global::shutdown) {
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
				console->info("io TAKEOFF");
				flightController.takeoff();
				break;
			case proto::RAW:
				console->info("io RAW");
				flightController.rawControl();
				break;
			case proto::VELOCITY:
				console->info("io VELOCITY");
				flightController.velocityControl();
				break;
			case proto::POSITION:
				console->info("io POSITION");
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
				camera.enablePosition();
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
			while(!io->recvMessage(update) && !global::shutdown) {}
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
	while (!global::shutdown) {
		protoObs.Clear();
		protoObs.set_battery(obs->skylineBattery);
		protoObs.set_velocity_x(obs->velocityX);
		protoObs.set_velocity_y(obs->velocityY);
		protoObs.set_position_x(obs->positionX);
		protoObs.set_position_y(obs->positionY);
		protoObs.set_position_z(obs->positionZ);
		protoObs.set_skyline_ang_roll_vel(obs->skylineAngRollVel);
		protoObs.set_skyline_ang_pitch_vel(obs->skylineAngPitchVel);
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
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

void do_serve(Watchdog &watchdog, Config &config,
		IOController &ioController,
		FlightController &flightController,
		std::shared_ptr<Observations> obs, Camera &camera) {
	while (!global::shutdown) {
		watchdog.stop();
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

void cdrone(Config &config) {

	// initialize the watchdogs
	console->info("initializing watchdogs");
	Watchdog updateWatchdog(config.updateWatchdog(), "update");
	Watchdog ioWatchdog(config.ioWatchdog(), "io");
	Watchdog controllerWatchdog(config.controllerWatchdog(), "controller");
	std::unique_ptr<VRPN> vrpn;
	
	// check for correct hardware
	if (std::thread::hardware_concurrency() != 4) {
		console->error("cpu not supported -- must have 4 hardware cores.");
		return;
	}

	// initialize the observations
	std::shared_ptr<Observations> obs = std::make_shared<Observations>();
	
	// initialize vrpn if enabled
	if (config.vrpnEnabled())
		vrpn = std::make_unique<VRPN>(config.vrpnName(), config.vrpnID(), obs);
	
	// initialize the hardware
	// console->info("initializing Infrared");
	// Infrared infrared(config, std::make_shared<Observations>());

	console->info("initializing FlightController");
	FlightController flightController(config, obs);

	console->info("initializing IOController");
	IOController ioController(config);

	console->info("initializing Camera");
	Camera camera(config, std::make_shared<Observations>());

	// start camera
	console->info("starting Camera");
	camera.start();

	// create threads
	console->info("creating threads");
	pthread_block(SIGINT);
	std::thread update_thr(do_update, std::ref(updateWatchdog), 
			/*std::ref(infrared),*/ std::ref(flightController));
	// std::thread controller_thr(do_controller, std::ref(controllerWatchdog), 
	// 		std::ref(config), std::ref(flightController));
	std::thread serve_thr(do_serve, std::ref(ioWatchdog), std::ref(config),
			std::ref(ioController), std::ref(flightController), obs,
			std::ref(camera));
	pthread_unblock(SIGINT);

	// move threads to specific cores
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(2, &cpuset);
	pthread_setaffinity_np(update_thr.native_handle(), sizeof(cpu_set_t), &cpuset);

	// CPU_ZERO(&cpuset);
	// CPU_SET(2, &cpuset);
	// pthread_setaffinity_np(controller_thr.native_handle(), sizeof(cpu_set_t), &cpuset);

	CPU_ZERO(&cpuset);
	CPU_SET(3, &cpuset);
	pthread_setaffinity_np(serve_thr.native_handle(), sizeof(cpu_set_t), &cpuset);
	
	// join with threads
	update_thr.join(); 
	console->info("update thread joined");
	serve_thr.join();
	console->info("io thread joined");
	// controller_thr.join();
	// console->info("controller thread joined");
	
	// stop camera
	camera.stop();
}

