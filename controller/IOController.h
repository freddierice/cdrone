#ifndef __IO_CONTROLLER_H__
#define __IO_CONTROLLER_H__
#include "proto/io.pb.h"
#include "misc/Config.h"
#include "wire/Server.h"

#include <stdexcept>
#include <memory>

class IOController {
public:
	IOController(Config &config);
	~IOController();
	
	std::unique_ptr<google::protobuf::Message> getMessage();

private:
	Server m_server;
};

#endif /* __IO_CONTROLLER_H__ */
