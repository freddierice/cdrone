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
	
	void getMessage(google::protobuf::Message& message);

private:
	Server m_server;
	google::protobuf::io::ZeroCopyInputStream* m_input;
	google::protobuf::io::ZeroCopyOutputStream* m_output;
};

#endif /* __IO_CONTROLLER_H__ */
