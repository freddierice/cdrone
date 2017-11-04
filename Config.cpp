#include "Config.h"

#include <iostream>
#include <fstream>

Config::Config(const std::string &filename) {
	std::ifstream config_file(filename);
	if (!config_file) { 
		throw ConfigException();
	}

	config_file >> this->m_root;
	m_name = this->m_root.get("name", "<no-name>").asString();
	m_infraredAlpha = this->m_root.get("infraredAlpha", 0.0).asDouble();
	m_infraredK = this->m_root.get("infraredK", 1.0).asDouble();
}
