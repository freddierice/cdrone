#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <spdlog/spdlog.h>

#include "main.h"
#include "controller/MultiWii.h"
#include "controller/Watchdog.h"
#include "hardware/Infrared.h"
#include "misc/Config.h"
#include "wire/Serial.h"
#include "wire/Server.h"

// test_infrared writes voltages and distances to stdout.
void test_infrared(Config &config) {
	spdlog::get("console")->info("initializing watchdog");
	Watchdog watchdog(1,0);
	
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
	Watchdog watchdog(1,0);

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
	spdlog::get("console")->info("initializing watchdog");
	Watchdog watchdog(1, 0);

	spdlog::get("console")->info("performing tasks");
	watchdog.start();
	for(int i = 0; i < 25; i++) {
		watchdog.ok();
		usleep(100000);
	}

	spdlog::get("console")->info("hanging");
	while (true) {}
}

void test_ssl(Config &config) try {

	char buf[6];
	Server server(config.port(), config.certificate(), config.key());
	server.accept();
	server.readAll(buf, 5);
	buf[5] = 0;
	spdlog::get("console")->info("recieved: {}", buf);

} catch(ServerException &ex) {
	spdlog::get("console")->error("could not start server: {}", ex.what());
}
