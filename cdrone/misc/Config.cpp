#include "Config.h"

#include <iostream>
#include <fstream>

Config::Config(const std::string &filename) {
	std::ifstream config_file(filename);
	if (!config_file) { 
		throw ConfigException("could not open file");
	}

	try {
		config_file >> m_root;
		
		m_name = m_root.get("name", "<no-name>").asString();
		m_infraredAlpha = m_root.get("infraredAlpha", 0.0).asDouble();
		m_infraredB = m_root.get("infraredB", 0.0).asDouble();
		m_infraredK = m_root.get("infraredK", 1.0).asDouble();
		auto updateWatchdog = m_root.get("updateWatchdog", 1).asInt();
		auto ioWatchdog = m_root.get("ioWatchdog", 1).asInt();
		auto analysisWatchdog = m_root.get("analysisWatchdog", 1).asInt();
		auto controllerWatchdog = m_root.get("controllerWatchdog", 1).asInt();
		m_updateWatchdog = std::chrono::milliseconds(updateWatchdog);
		m_ioWatchdog = std::chrono::milliseconds(ioWatchdog);
		m_analysisWatchdog = std::chrono::milliseconds(analysisWatchdog);
		m_controllerWatchdog = std::chrono::milliseconds(controllerWatchdog);
		m_key = m_root.get("key", "key.pem").asString();
		m_certificate = m_root.get("ca", "ca.pem").asString();
		int port = m_root.get("port", 6006).asInt();
		if (port > 0xFFFF || port < 0)
			throw ConfigException("invalid port number");
		m_port = (uint16_t) port;
		m_ssl = m_root.get("ssl", false).asBool();
		m_skylinePort = m_root.get("skylinePort", "/dev/ttyUSB0").asString();
		m_cameraPort = m_root.get("cameraPort", "/dev/nyi").asString();
	} catch(Json::RuntimeError &e) {
		throw ConfigException(e.what());
	}
}
