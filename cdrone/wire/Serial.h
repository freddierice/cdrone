#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <string>

class SerialException : public std::exception {};

class Serial {
public:
	Serial(const std::string& filename);

	// read full from the line. if n bytes are not read,
	// the function returns false.
	bool readFull(void* buf, int n);
	
	// writeFull writes a full n characters.
	bool writeFull(const void* buffer, int n);
private:
	static int serialOpen(const std::string& filename);
	int m_fd;
	const std::string& m_filename;
};

#endif /* __SERIAL_H__ */
