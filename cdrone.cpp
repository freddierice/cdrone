#include <iostream>
#include <unistd.h>

#include "config.h"
#include "watchdog.h"

int main(int argc, const char *argv[]) {
	try {
		Config config("cdrone.conf");
	} catch ( ConfigException& e) {
		std::cerr << "could not parse config file" << std::endl;
		std::exit(1);
	}
	
	Watchdog watchdog(3,0);

	watchdog.start();
	for(;;) {
		watchdog.ok();
		sleep(1);
	}
	return 0;
}
