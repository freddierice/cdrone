#include "Config.h"

#include <iostream>
#include <fstream>

Config::Config(const std::string &filename) {
	std::ifstream config_file(filename);
	if (!config_file) { 
		throw ConfigException("could not open file");
	}

	try {
		config_file >> this->m_root;
		m_name = this->m_root.get("name", "<no-name>").asString();
		m_infraredAlpha = this->m_root.get("infraredAlpha", 0.0).asDouble();
		m_infraredB = this->m_root.get("infraredB", 0.0).asDouble();
		m_infraredK = this->m_root.get("infraredK", 1.0).asDouble();
		m_updateWatchdog = this->m_root.get("updateWatchdog", 1).asInt();
		m_ioWatchdog = this->m_root.get("ioWatchdog", 1).asInt();
		m_analysisWatchdog = this->m_root.get("analysisWatchdog", 1).asInt();
		m_controllerWatchdog = this->m_root.get("controllerWatchdog", 1).asInt();
		m_key = this->m_root.get("key", "key.pem").asString();
		m_certificate = this->m_root.get("ca", "ca.pem").asString();
		int port = this->m_root.get("port", 6006).asInt();
		if (port > 0xFFFF || port < 0)
			throw ConfigException("invalid port number");
		m_port = (uint16_t) port;
	} catch(Json::RuntimeError &e) {
		throw ConfigException(e.what());
	}
}
