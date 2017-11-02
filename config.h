#ifndef __CONFIG_H__
#define __CONFIG_H_
#include <string>

#include "json/json.h"

class ConfigException : public std::exception {};

// Config holds configurations that are not needed at compile time.
class Config {
public:
	Config(const std::string &filename);

	std::string Name();
private:
	Json::Value m_root;
};

#endif /* __CONFIG_H__ */
