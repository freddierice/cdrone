#include "config.h"

#include <iostream>
#include <fstream>

Config::Config(const std::string &filename) {
	std::ifstream config_file(filename);
	if (!config_file) { 
		throw ConfigException();
	}

	config_file >> this->m_root;
}

std::string Config::Name() {
	return this->m_root.get("name", "<no-name>").asString();
}
