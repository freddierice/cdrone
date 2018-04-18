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
	const int& cameraPort() const { return m_cameraPort; }
	const double& rollPIDP() const {return m_rollPIDP; }
	const double& rollPIDI() const {return m_rollPIDI; }
	const double& rollPIDD() const {return m_rollPIDD; }
	const double& pitchPIDP() const {return m_pitchPIDP; }
	const double& pitchPIDI() const {return m_pitchPIDI; }
	const double& pitchPIDD() const {return m_pitchPIDD; }
	const double& throttlePIDP() const {return m_throttlePIDP; }
	const double& throttlePIDI() const {return m_throttlePIDI; }
	const double& throttlePIDD() const {return m_throttlePIDD; }
	const std::string& vrpnName() const {return m_vrpnName; }
	const uint32_t& vrpnID() const {return m_vrpnID; }
	const bool& vrpnEnabled() const {return m_vrpnEnabled; }
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
	int m_cameraPort;
	double m_rollPIDP;
	double m_rollPIDI;
	double m_rollPIDD;
	double m_pitchPIDP;
	double m_pitchPIDI;
	double m_pitchPIDD;
	double m_throttlePIDP;
	double m_throttlePIDI;
	double m_throttlePIDD;
	std::string m_vrpnName;
	uint32_t m_vrpnID;
	bool m_vrpnEnabled;
};

#endif /* __CONFIG_H__ */
