#ifndef __CONFIG_H__
#define __CONFIG_H_
#include <string>

#include "json/json.h"

class ConfigException : public std::exception {};

// Config holds configurations that are not needed at compile time.
class Config {
public:
	Config(const std::string &filename);

	const double& infraredAlpha() const { return m_infraredAlpha; }
	const double& infraredB() const { return m_infraredB; }
	const double& infraredK() const { return m_infraredK; }
	const std::string& name() const { return m_name; }
	const time_t& updateWatchdog() const { return m_updateWatchdog; }
	const time_t& ioWatchdog() const { return m_ioWatchdog; }
	const time_t& analysisWatchdog() const { return m_analysisWatchdog; }
	const time_t& controllerWatchdog() const { return m_controllerWatchdog; }
private:
	Json::Value m_root;
	std::string m_name;
	double m_infraredAlpha;
	double m_infraredB;
	double m_infraredK;
	time_t m_updateWatchdog;
	time_t m_ioWatchdog;
	time_t m_analysisWatchdog;
	time_t m_controllerWatchdog;
};

#endif /* __CONFIG_H__ */
