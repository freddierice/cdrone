#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <string>

#include "misc/exception.h"

CDRONE_EXCEPTION(HardwareException, SerialException);

// Serial reads and writes from a serial port.
class Serial {
public:
	// open a serial port with the filesystem node.
	Serial(const std::string& filename);

	// read will try to read at least n characters from the line.
	int read(void* buf, int n);

	// write will try to write all the characters
	int write(const void* buf, int n);

	// read full n characters from the line. if n bytes are not read, the
	// function throws a SerialException.
	bool readFull(void* buf, int n);
	
	// writeFull writes a full n characters. If n characters are not read, the
	// function will throw a SerialException.
	void writeFull(const void* buffer, int n);
private:
	static int serialOpen(const std::string& filename);
	
	// m_fd holds the file descriptor for the serial port.
	int m_fd;

	// m_filename is the filesystem node for the serial port.
	const std::string& m_filename;
};

#endif /* __SERIAL_H__ */
