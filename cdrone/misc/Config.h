#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <chrono>
#include <string>

#include "json/json.h"

class ConfigException : public std::runtime_error {
public:
	ConfigException(std::string const& msg) : std::runtime_error(msg) {}
};

// Config holds configurations that are not needed at compile time.
class Config {
public:
	Config(const std::string &filename);

	const double& infraredAlpha() const { return m_infraredAlpha; }
	const double& infraredB() const { return m_infraredB; }
	const double& infraredK() const { return m_infraredK; }
	const std::string& name() const { return m_name; }
	const std::chrono::milliseconds& updateWatchdog() const { return m_updateWatchdog; }
	const std::chrono::milliseconds& ioWatchdog() const { return m_ioWatchdog; }
	const std::chrono::milliseconds& analysisWatchdog() const { return m_analysisWatchdog; }
	const std::chrono::milliseconds& controllerWatchdog() const { return m_controllerWatchdog; }
	const std::string& certificate() const { return m_certificate; }
	const std::string& key() const { return m_key; }
	const uint16_t& port() const { return m_port; }
	const bool ssl() const { return m_ssl; }
	const std::string& skylinePort() const { return m_skylinePort; }
	const std::string& cameraPort() const { return m_cameraPort; }
private:
	Json::Value m_root;
	
	std::string m_name;
	std::string m_key;
	std::string m_certificate;
	double m_infraredAlpha;
	double m_infraredB;
	double m_infraredK;
	std::chrono::milliseconds m_updateWatchdog;
	std::chrono::milliseconds m_ioWatchdog;
	std::chrono::milliseconds m_analysisWatchdog;
	std::chrono::milliseconds m_controllerWatchdog;
	uint16_t m_port;
	bool m_ssl;
	std::string m_skylinePort;
	std::string m_cameraPort;
};

#endif /* __CONFIG_H__ */
