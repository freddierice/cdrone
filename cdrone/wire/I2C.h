#ifndef __I2C_H__
#define __I2C_H__
#include <string>
#include <inttypes.h>

#include "misc/exception.h"

CDRONE_EXCEPTION(HardwareException, I2CException);

class I2C {
public:
	I2C(int addr);
	I2C(int addr, const std::string &filename);
	~I2C();

	// write value to reg.
	void writeRegister(uint8_t reg, uint16_t value);

	// read a value from reg.
	uint16_t readRegister(uint8_t reg);
	
	// default I2C filename
	static const std::string I2C_FILENAME;
private:
	int m_addr;
	int m_fd;

	// open an I2C connection, returns a file descriptor
	static int openI2C(const std::string &filename, int addr);
};

#endif /* __I2C_H__ */
