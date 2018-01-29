#include <sys/socket.h>
#include <sys/types.h>

#include "misc/logging.h"
#include "misc/utility.h"
#include "wire/IO.h"

IO::IO(int fd) : m_fd(fd), m_recvBufferLen(4096), m_sendBufferLen(4096),
	m_recvBuffer(new char[m_recvBufferLen]),
	m_sendBuffer(new char[m_sendBufferLen]),
	m_recvBufferIter(m_recvBuffer), m_recvLen(0), m_connected(true) {
	
	// set timeout to 100 ms.
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval)) < 0)
		throw IOException("could not set timeout on socket");
}

IO::~IO() {
	m_connected = false;
	delete m_recvBuffer;
	delete m_sendBuffer;
}

[[ noreturn ]] void IO::disconnect(const std::string& msg) {
	m_connected = false;
	throw IOException(msg);
}

bool IO::connected() {
	return m_connected;
}

void IO::sendMessage(google::protobuf::Message &message) {
	uint32_t len;
	std::string data;

	if (!m_connected)
		throw IOException("disconnected");

	// write the length of the message
	len = message.ByteSize();
	if (::writeFull(m_fd, (const char *)&len, sizeof(len)) < 0)
		disconnect("could not write length of message");
	
	if (len > m_sendBufferLen) {
		m_sendBufferLen = len;
		delete m_sendBuffer;
		m_sendBuffer = new char[m_sendBufferLen];
	}
	if (!message.SerializeToArray(m_sendBuffer, (int)len))
		disconnect("could not write message to an array");
	if (::writeFull(m_fd, (const char *)m_sendBuffer, (int)len) < 0)
		disconnect("could not write message to the wire");
}

bool IO::recvMessage(google::protobuf::Message &message) {
	int ret;
	if (!m_connected)
		throw IOException("disconnected");

	if (m_recvLen == 0) {
again1:
		if ((ret = ::read(m_fd, m_recvBufferIter, 
						4 + m_recvBuffer - m_recvBufferIter)) < 0) {
			if (errno == EINTR)
				goto again1;
			if (errno == EAGAIN)
				return false;
			disconnect("could not read length from socket");
		}
		m_recvBufferIter += ret;
		if (m_recvBufferIter - m_recvBuffer < 4)
			return false;
		m_recvBufferIter = m_recvBuffer;
		m_recvLen = *(uint32_t *)m_recvBuffer;
		if (m_recvLen == 0)
			return true; // 0 means empty (default) protobuf.
	}
		
again2:
	if ((ret = ::read(m_fd, m_recvBufferIter, 
					m_recvLen + m_recvBuffer - m_recvBufferIter)) < 0) {
		if (errno == EINTR)
			goto again2;
		if (errno == EAGAIN)
			return false;
		disconnect("could not read length from socket");
	}
	m_recvBufferIter += ret;

	if (m_recvBufferIter - m_recvBuffer != (int)m_recvLen)
		return false;

	if (!message.ParseFromArray(m_recvBuffer, m_recvLen))
		disconnect("could not read in ParseFromArray");

	m_recvBufferIter = m_recvBuffer;
	m_recvLen = 0;

	return true;
}
