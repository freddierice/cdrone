#include <atomic>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <spdlog/spdlog.h>

#include "Config.h"
#include "Watchdog.h"
#include "Serial.h"
#include "MultiWii.h"
#include "Infrared.h"
#include "main.h"

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
