#include <iostream>
#include <unistd.h>
#include <string.h>

#include "Config.h"
#include "Watchdog.h"
#include "Serial.h"
#include "MultiWii.h"
#include "Infrared.h"

int main(int argc, const char *argv[]) try {

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

	Infrared infrared;
	for (;;) {
		std::cout << infrared.distance() << std::endl;
	}

	return 0;

	Config config("cdrone.conf");
	Watchdog watchdog(3,0);

	watchdog.start();
	for(;;) {
		watchdog.ok();
		sleep(1);
		std::cout << "Name: " << config.Name() << std::endl;
	}
	return 0;
} catch (ConfigException& ex) {
	std::cerr << "could not parse config file" << std::endl;
	return 1;
}
