#include "I2C.h"

#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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
	uint16_t val;
	if (write(this->m_fd, &reg, 1)==-1) {
		// TODO: check for interrupt.
		throw I2CException();
	}

	// purposeful block, we should get a response soon..
	if (read(this->m_fd, &val, 2)==-1) {
		// TODO: check for interrupt.
	}
	return ntohs(val);
}

void I2C::writeRegister(uint8_t reg, uint16_t value) {
	char buf[3];
	
	buf[0] = (char)reg;
	*(uint16_t*)(buf + 1) = htons(value);
	if (write(this->m_fd, buf, 3) == -1) {
		// TODO: check for interrupt.
		throw I2CException();
	}
	// XXX: need ioctl/flush?
}

int I2C::openI2C(const std::string &filename, int addr) {
	int fd;
	if ((fd = open(filename.c_str(), O_RDWR)) == -1) {
		throw I2CException();
	}
	
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		std::cerr << "could not set I2C_SLAVE" << std::endl;
		throw I2CException();
	}

	return fd;
}

const std::string I2C::I2C_FILENAME =  "/dev/i2c-1";
