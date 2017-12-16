#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "main.h"
#include "controller/Watchdog.h"
#include "hardware/Camera.h"
#include "hardware/Infrared.h"
#include "hardware/Skyline.h"
#include "misc/Config.h"
#include "misc/logging.h"
#include "wire/MultiWii.h"
#include "wire/Serial.h"
#include "wire/Server.h"

// test_infrared writes voltages and distances to stdout.
void test_infrared(Config &config) {
	console->info("initializing watchdog");
	Watchdog watchdog(std::chrono::seconds(1));
	
	console->info("initializing infrared");
	auto obs = std::make_shared<Observations>();
	Infrared infrared(config, obs);
	
	console->info("starting loop");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		infrared.update();
		console->info("height: {}, voltage: {}", obs->infraredHeight,
				obs->infraredVoltage);
	}
}

void test_multiwii(Config &config) {
	console->info("initializing watchdog");
	Watchdog watchdog(std::chrono::seconds(1));

	console->info("initializing serial");
	Serial serial(config.skylinePort());

	console->info("initializing multiwii");
	MultiWii m(serial);

	console->info("starting loop");
	watchdog.start();
	m.sendCMD(MultiWiiCMD::MSP_STATUS);
	while (!shutdown) {
		watchdog.ok();
		// TODO: add multiwii tests
	}
}

void test_watchdog(Config &config) {
	console->info("initializing watchdog1");
	Watchdog watchdog1(std::chrono::seconds(1));
	console->info("initializing watchdog2");
	Watchdog watchdog2(std::chrono::seconds(1));

	console->info("starting watchdog1");
	watchdog1.start();
	for(int i = 0; i < 25; i++) {
		watchdog1.ok();
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
	console->info("stopping watchdog1");
	watchdog1.stop();

	console->info("starting watchdog2");
	watchdog2.start();
	for (int i = 0; i < 25; i++) {
		watchdog2.ok();
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	console->info("starting watchdog1 again");
	watchdog1.start();
	for (int i = 0; i < 25; i++) {
		watchdog1.ok();
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		watchdog2.ok();
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

	console->info("stopping watchdog1");
	watchdog1.stop();

	console->info("hanging on watchdog2");
	while (true) {}
}

void test_ssl(Config &config) try {

	/*
	const void *buf;
	int len;
	Server server(config);

	auto zeroCopyStreams = server.accept();
	auto zeroInputStream = zeroCopyStreams.first;
	while (zeroInputStream->ByteCount() < 10 && 
			zeroInputStream->Next(&buf, &len)) {
		std::string recieved((const char *)buf, (size_t)len);
		console->info("recieved: {}", recieved);
	}
	*/

} catch(ServerException &ex) {
	console->error("could not start server: {}", ex.what());
}

void test_camera(Config &config) {
	double xMotion, yMotion, xPosition, yPosition;

	auto obs = std::make_shared<Observations>();
	try {
		console->info("initializing camera");
		Camera camera(config, obs);

		console->info("starting camera");
		camera.start();

		console->info("letting camera warm up");
		std::this_thread::sleep_for(std::chrono::seconds(1));

		console->info("starting position mode");
		camera.enablePosition();
		while (!shutdown) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			xMotion= obs->cameraVelocityX;
			yMotion = obs->cameraVelocityY;
			xPosition = obs->cameraPositionX;
			yPosition = obs->cameraPositionY;

			console->info("motion: {}, {}", xMotion, yMotion);
			console->info("position: {}, {}", xPosition, yPosition);
		}

		console->info("stopping camera");
		camera.stop();
	} catch (HardwareException &ex) {
		console->error("error with camera: {}", ex.what());
	}
}

void test_calibrate(Config &config) {
	try {
		console->info("initializing skyline");
		auto obs = std::make_shared<Observations>();
		Skyline skyline(config, obs);

		console->info("sending calibrate");
		skyline.sendCalibrate();
		while (!shutdown && !skyline.calibrateDone()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			skyline.update();
		}
		console->info("got calibrate");
	} catch (HardwareException &ex) {
		console->error("got a hardware exception: {}", ex.what());
	}
}

void test_skyline(Config &config) {
	try {
		console->info("initializing skyline");
		auto obs = std::make_shared<Observations>();
		Skyline skyline(config, obs);
		
		console->info("starting info loop");
		skyline.sendCalibrate();
		// skyline.sendAttitude();
		// skyline.sendIMU();
		// skyline.sendAnalog();
		while (!shutdown) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			skyline.update();
			console->info("vbat: {}, heading: {}", obs->skylineBattery,
					obs->skylineAngYaw);
		}

	} catch (HardwareException &ex) {
		console->error("got a hardware exception: {}", ex.what());
	}
}
