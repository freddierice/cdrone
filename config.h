#ifndef __CONFIG_H__
#define __CONFIG_H_
#include <string>

class ConfigException : public std::exception {
	 const char * what() const throw() {
		return "Config Exception";
	 }
};

// Config holds configurations that are not needed at compile time.
class Config {
public:
	Config(const std::string &filename);
private:
	const std::string &m_filename;
};

#endif /* __CONFIG_H__ */
