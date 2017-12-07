#ifndef __IO_CONTROLLER_H__
#define __IO_CONTROLLER_H__
#include "proto/io.pb.h"
#include "misc/Config.h"
#include "wire/Server.h"

#include <stdexcept>
#include <memory>

#include <google/protobuf/io/zero_copy_stream.h>

class IOControllerException : public std::runtime_error {
public:
	IOControllerException(std::string const& msg) : std::runtime_error(msg) {}
};

class IOController {
public:
	IOController(Config& config);
	~IOController();
	
	void accept();
	void getMessage(google::protobuf::Message& message);

	// true after a successful accept.
	bool connected();

private:
	Server m_server;
	std::unique_ptr<google::protobuf::io::CodedInputStream> m_input;
	std::unique_ptr<google::protobuf::io::CodedOutputStream> m_output;
	char* m_buffer;
	uint64_t m_bufferLen;
	bool m_connected;
};

#endif /* __IO_CONTROLLER_H__ */
