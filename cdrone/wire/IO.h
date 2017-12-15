#ifndef __IO_H__
#define __IO_H__
#include <memory>

#include "misc/Config.h"
#include "misc/exception.h"
#include "misc/Observations.h"
#include "proto/io.pb.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

CDRONE_EXCEPTION(HardwareException, IOException);

class IO {
public:
	IO(int fd);
	~IO();
	
	// send/recv a protobuf, recv returns true if a message is returned.
	void sendMessage(google::protobuf::Message& message);
	bool recvMessage(google::protobuf::Message& message);

	// parseVarint is pretty much copy-pastad from protobuf coded_stream.cc
	// I've included it here because it is not exported.
	static bool readVarint(void *data, int size, uint64_t *value);

	// true while the connection is valid.
	bool connected();

private:
	// no default constructor, only valid with connection.
	IO() = delete;

	// disconnect and throw
	[[ noreturn]] void disconnect(const std::string& msg);

	int m_fd;
	uint32_t m_recvBufferLen, m_sendBufferLen;
	char *m_recvBuffer, *m_sendBuffer;
	char *m_recvBufferIter;
	uint32_t m_recvLen;
	std::atomic<bool> m_connected;
};

#endif /* __IO_CONTROLLER_H__ */
