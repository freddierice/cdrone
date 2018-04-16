#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "logging/logging.h"
#include "wire/Serial.h"


Serial::Serial(const std::string& filename) : m_filename(filename) {
	m_fd = serialOpen(filename);
}

int Serial::read(void* buf, int n) {
	int ret;

again:
	if ((ret = ::read(m_fd, buf, n)) < 0) {
		// got interrupted, try again.
		if (errno == EINTR)
			goto again;
		// read call would block, let caller know.
		if (errno == EAGAIN)
			return 0;
		throw Serial("could not read");
	}

	return ret;
}

int Serial::write(const void* buf, int n) {
	int ret;

again:
	if ((ret = ::write(m_fd, buf, n)) < 0) {
		if (errno == EINTR)
			goto again;
		throw Serial("could not write");
	}

	return ret;
}

void Serial::readFull(void *buf, int n) {
	int numRead = 0;

	while (numRead < n) {
		int ret;
		if ((ret = ::read(m_fd, (void*)((char*)buf+numRead), n-numRead)) < 0) {
			// if interrupt or would block, try again.
			if (errno == EINTR || errno == EAGAIN)
				continue;
			throw SerialException("could not read from the serial line");
		}
		numRead += ret;
	}
}

void Serial::writeFull(const void *buf, int n) {
	int total = 0;

	do {
		total += write((const void *)((char *)buf+total), n-total);
	} while (total != n);
}

void Serial::flush() {
	::tcdrain(m_fd);
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

	// turn off all special processing
	cfmakeraw(&tty);

	// set baud rate
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	// read doesn't block
	tty.c_cc[VMIN] = 0;

	// read timeout (non-blocking)
	tty.c_cc[VTIME] = 0; 

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		throw SerialException("could not set tc attributes");
	}

	return fd;
}
