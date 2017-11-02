#include <iostream>
#include <unistd.h>

#include "config.h"
#include "watchdog.h"

int main(int argc, const char *argv[]) try {
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
