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
	const double& rollPIDP() const { return m_rollPIDP; }
	const double& rollPIDI() const { return m_rollPIDI; }
	const double& rollPIDD() const { return m_rollPIDD; }
	const double& pitchPIDP() const { return m_pitchPIDP; }
	const double& pitchPIDI() const { return m_pitchPIDI; }
	const double& pitchPIDD() const { return m_pitchPIDD; }
	const double& throttlePIDP() const { return m_throttlePIDP; }
	const double& throttlePIDI() const { return m_throttlePIDI; }
	const double& throttlePIDD() const { return m_throttlePIDD; }
	const double& throttlePIDK() const { return m_throttlePIDK; }
	const double& positionXPIDP() const { return m_positionXPIDP; }
	const double& positionXPIDI() const { return m_positionXPIDI; }
	const double& positionXPIDD() const { return m_positionXPIDD; }
	const double& positionXPIDK() const { return m_positionXPIDK; }
	const double& positionXPIDCenter() const { return m_positionXPIDCenter; }
	const double& positionYPIDP() const { return m_positionYPIDP; }
	const double& positionYPIDI() const { return m_positionYPIDI; }
	const double& positionYPIDD() const { return m_positionYPIDD; }
	const double& positionYPIDK() const { return m_positionYPIDK; }
	const double& positionYPIDCenter() const { return m_positionYPIDCenter; }
	const double& positionZPIDP() const { return m_positionZPIDP; }
	const double& positionZPIDI() const { return m_positionZPIDI; }
	const double& positionZPIDD() const { return m_positionZPIDD; }
	const double& positionZPIDK() const { return m_positionZPIDK; }
	const double& positionZPIDCenter() const { return m_positionZPIDCenter; }
	const std::string& vrpnName() const { return m_vrpnName; }
	const uint32_t& vrpnID() const { return m_vrpnID; }
	const bool& vrpnEnabled() const { return m_vrpnEnabled; }
	const double& maxPositionDeltaX() const { return m_maxPositionDeltaX; }
	const double& maxPositionDeltaY() const { return m_maxPositionDeltaY; }
	const double& maxPositionDeltaZ() const { return m_maxPositionDeltaZ; }
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
	double m_throttlePIDK;
	double m_positionXPIDP;
	double m_positionXPIDI;
	double m_positionXPIDD;
	double m_positionXPIDK;
	double m_positionXPIDCenter;
	double m_positionYPIDP;
	double m_positionYPIDI;
	double m_positionYPIDD;
	double m_positionYPIDK;
	double m_positionYPIDCenter;
	double m_positionZPIDP;
	double m_positionZPIDI;
	double m_positionZPIDD;
	double m_positionZPIDK;
	double m_positionZPIDCenter;
	std::string m_vrpnName;
	uint32_t m_vrpnID;
	bool m_vrpnEnabled;
	double m_maxPositionDeltaX;
	double m_maxPositionDeltaY;
	double m_maxPositionDeltaZ;
};

#endif /* __CONFIG_H__ */
