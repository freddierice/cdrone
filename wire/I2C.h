#ifndef __I2C_H__
#define __I2C_H__
#include <string>
#include <inttypes.h>

class I2CException : public std::exception {};

class I2C {
public:
	I2C(int addr);
	I2C(int addr, const std::string &filename);
	~I2C();

	// bool read(int cmd);
	void writeRegister(uint8_t reg, uint16_t value);
	uint16_t readRegister(uint8_t reg);
	// bool write(int cmd, const char *buf, int n);
	
	static const std::string I2C_FILENAME;
private:
	int m_addr;
	int m_fd;

	static int openI2C(const std::string &filename, int addr);
};

#endif /* __I2C_H__ */
