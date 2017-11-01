#include "config.h"

#include <iostream>
#include <fstream>

#include "json/json.h"

Config::Config(const std::string &filename) : m_filename(filename) {
	std::ifstream config_file(filename);
	if (!config_file) { 
		std::cerr << "could not open config file" << std::endl;
	}

	// TODO: get json stuff here.
}
