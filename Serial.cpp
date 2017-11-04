#include "Serial.h"

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

Serial::Serial(const std::string& filename) : m_filename(filename) {
	this->m_fd = serialOpen(filename);
}

bool Serial::readFull(char *buf, int n) {
	int num_read;
	if (::ioctl(this->m_fd, FIONREAD, &num_read)) {
		std::cerr << "could not ioctl tty" << std::endl;

		// try to recover
		::close(this->m_fd);
		serialOpen(this->m_filename);
		if (::ioctl(this->m_fd, FIONREAD, &num_read)) {
			throw SerialException();
		}
	}

	if (num_read < n)
		return false;

	if ((num_read = read(this->m_fd, buf, n)) != n) {
		// TODO: handle interrupts.
		std::cerr << "wrong number of bytes read. expected " << n << " got " << 
			num_read << std::endl;
		throw SerialException();
	}

	return true;
}

bool Serial::writeFull(const char *buf, int n) {
	int total;
	if ((total = write(this->m_fd, buf, n)) != n) {
		if (total == -1) {
			std::cerr << "error writing" << std::endl;
			throw SerialException();
		}
		// TODO: handle interrupts.
		std::cerr << "wrong number of bytes read. expected " << n << " got " << 
			total << std::endl;
		throw SerialException();
	}

	// flush written to m_fd to the wire.
	tcflush(this->m_fd, TCOFLUSH);
	return true;
}

int Serial::serialOpen(const std::string& filename) {
	struct termios tty;
	int fd;
	if ((fd = ::open(filename.c_str(), O_RDWR | O_NOCTTY | O_SYNC)) == -1) {
		throw SerialException();
	}

	if (tcgetattr(fd, &tty)) {
		throw SerialException();
	}

	cfmakeraw(&tty);

	// set baud rate
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	// read doesn't block
	tty.c_cc[VMIN] = 0;

	// 0.3 second read timeout
	tty.c_cc[VTIME] = 3; 

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		throw SerialException();
	}

	return fd;
}
