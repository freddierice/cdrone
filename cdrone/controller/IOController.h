#ifndef __IO_CONTROLLER_H__
#define __IO_CONTROLLER_H__
#include <memory>

#include "misc/Config.h"
#include "misc/exception.h"
#include "misc/Observations.h"
#include "proto/io.pb.h"
#include "wire/Server.h"
#include "wire/IO.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

CDRONE_EXCEPTION(ControllerException, IOControllerException);

class IOController {
public:
	IOController(Config &config);
	~IOController();
	
	std::shared_ptr<IO> accept();

private:
	Server m_server;
};

#endif /* __IO_CONTROLLER_H__ */
