#include <iostream>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "wire/I2C.h"

I2C::I2C(int addr) : m_addr(addr) {
	m_fd = openI2C(I2C_FILENAME, addr);
}

I2C::I2C(int addr, const std::string &filename) : m_addr(addr) {
	m_fd = openI2C(filename, addr);
}

I2C::~I2C() {
	if (m_fd > 0) ::close(m_fd);
}

uint16_t I2C::readRegister(uint8_t reg) {
	uint16_t value;

reg_again:
	if (::write(m_fd, (const void *)&reg, sizeof(reg)) < 0) {
		if (errno == EINTR)
			goto reg_again;
		throw I2CException("could not write register to i2c");
	}

	// purposeful block, we should get a response soon.
value_again:
	if (::read(m_fd, &value, 2)==-1) {
		if (errno == EINTR)
			goto value_again;
		throw I2CException("could not read from i2c");
	}

	return ntohs(value);
}

void I2C::writeRegister(uint8_t reg, uint16_t value) {
	struct message {
		uint8_t  reg;
		uint16_t value;
	} __attribute__((packed)) msg;
	msg.reg = reg;
	msg.value = htons(value);

again:
	if (::write(m_fd, &msg, sizeof(msg)) < 0) {
		if (errno == EINTR)
			goto again;
		throw I2CException("could not write value to i2c");
	}
}

int I2C::openI2C(const std::string &filename, int addr) {
	int fd;

	if ((fd = ::open(filename.c_str(), O_RDWR)) == -1)
		throw I2CException("could not open i2c");
	if (::ioctl(fd, I2C_SLAVE, addr) < 0)
		throw I2CException("could not set I2C_SLAVE");

	return fd;
}

const std::string I2C::I2C_FILENAME =  "/dev/i2c-1";
