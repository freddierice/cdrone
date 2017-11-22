#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <spdlog/spdlog.h>

#include "main.h"
#include "controller/Watchdog.h"
#include "hardware/Infrared.h"
#include "misc/Config.h"
#include "wire/MultiWii.h"
#include "wire/Serial.h"
#include "wire/Server.h"

// test_infrared writes voltages and distances to stdout.
void test_infrared(Config &config) {
	spdlog::get("console")->info("initializing watchdog");
	Watchdog watchdog(std::chrono::seconds(1));
	
	spdlog::get("console")->info("initializing infrared");
	Infrared infrared(config.infraredAlpha(), config.infraredB(), 
			config.infraredK());
	
	spdlog::get("console")->info("starting loop");
	watchdog.start();
	while (!shutdown) {
		watchdog.ok();
		infrared.update();
		std::cout << infrared.voltage() << " " << infrared.distance() << std::endl;
	}
}

void test_multiwii(Config &config) {
	spdlog::get("console")->info("initializing watchdog");
	Watchdog watchdog(std::chrono::seconds(1));

	spdlog::get("console")->info("initializing serial");
	Serial serial("/dev/ttyUSB0");
	spdlog::get("console")->info("initializing multiwii");
	MultiWii m(serial);

	spdlog::get("console")->info("starting loop");
	watchdog.start();
	m.sendCMD(MultiWiiCMD::MSP_STATUS);
	while (!shutdown) {
		watchdog.ok();
		char b;
		if (serial.readFull(&b, 1)) {
			std::cout << b;
			std::cout.flush();
		}
	}
}

void test_watchdog(Config &config) {
	auto console = spdlog::get("console");
	
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

	const void *buf;
	int len;
	Server server(config);
	auto zeroCopyStreams = server.accept();
	auto zeroInputStream = zeroCopyStreams.first;
	while (zeroInputStream->ByteCount() < 10 && 
			zeroInputStream->Next(&buf, &len)) {
		std::string recieved((const char *)buf, (size_t)len);
		spdlog::get("console")->info("recieved: {}", recieved);
	}

} catch(ServerException &ex) {
	spdlog::get("console")->error("could not start server: {}", ex.what());
}
