#include "Serial.h"

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#include <spdlog/spdlog.h>

Serial::Serial(const std::string& filename) : m_filename(filename) {
	m_fd = serialOpen(filename);
}

int Serial::read(void* buf, int n) {
	int ret = ::read(m_fd, buf, n);
	if (ret < 0) 
		throw Serial("could not read");
	return ret;
}

int Serial::write(const void* buf, int n) {
	int ret = ::write(m_fd, buf, n);
	if (ret < 0) 
		throw Serial("could not write");
	return ret;
}

bool Serial::readFull(void *buf, int n) {
	int nRead;
	if (::ioctl(m_fd, FIONREAD, &nRead)) {
		spdlog::get("console")->warn("could not ioctl tty");

		// try to recover
		::close(m_fd);
		serialOpen(m_filename);
		if (::ioctl(m_fd, FIONREAD, &nRead)) {
			throw SerialException("error getting unread buffer");
		}
	}

	if (nRead < n)
		return false;

	if ((nRead = ::read(m_fd, buf, n)) != n) {
		// TODO: handle interrupts.
		spdlog::get("console")->error(
				"wrong number of bytes read. expected {} got {}", n, nRead);
		throw SerialException("wrong number of bytes read");
	}

	return true;
}

void Serial::writeFull(const void *buf, int n) {
	int total;
	if ((total = ::write(m_fd, buf, n)) != n) {
		if (total == -1)
			throw SerialException("error writing");
		// TODO: handle interrupts.
		spdlog::get("console")->error(
				"wrong number of bytes written. expected {} got {}", n, total);
		throw SerialException("wrong number of bytes written");
	}

	// flush written to m_fd to the wire.
	tcflush(m_fd, TCOFLUSH);
}

int Serial::serialOpen(const std::string& filename) {
	struct termios tty;
	int fd;
	if ((fd = ::open(filename.c_str(), O_RDWR | O_NOCTTY | O_SYNC)) == -1) {
		throw SerialException("could not open the serial port");
	}

	if (tcgetattr(fd, &tty)) {
		throw SerialException("could not get tc attributes");
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
		throw SerialException("could not set tc attributes");
	}

	return fd;
}
