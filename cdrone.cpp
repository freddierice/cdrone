#include <iostream>
#include <atomic>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "Config.h"
#include "Watchdog.h"
#include "Serial.h"
#include "MultiWii.h"
#include "Infrared.h"

std::atomic_bool shutdown;

void shutdown_handler(int sig) {
	shutdown = true;
	std::cerr << "SIGINT -- shutting down" << std::endl;
}

int main(int argc, const char *argv[]) try {

	// install signal handler
	signal(SIGINT, shutdown_handler);

	/*
	Serial serial("/dev/ttyUSB0");
	MultiWii m(serial);
	m.sendCMD(MultiWiiCMD::MSP_STATUS);
	for (;;) {
		char b;
		if (serial.readFull(&b, 1)) {
			std::cout << b;
			std::cout.flush();
		}
	}
	*/

	Config config("cdrone.conf");
	std::cout << config.infraredAlpha() << " " << config.infraredK() << std::endl;
	Infrared infrared(config.infraredAlpha(), config.infraredB(), 
			config.infraredK());
	while(!shutdown) {
		infrared.update();
		//std::cout << infrared.raw() << std::endl;
		std::cout << infrared.voltage() << " " << infrared.distance() << std::endl;
	}

	std::cerr << "shutting down" << std::endl;

	return 0;

	Watchdog watchdog(3,0);

	watchdog.start();
	for(;;) {
		watchdog.ok();
		sleep(1);
		std::cout << "Name: " << config.name() << std::endl;
	}
	return 0;
} catch (ConfigException& ex) {
	std::cerr << "could not parse config file" << std::endl;
	return 1;
}
