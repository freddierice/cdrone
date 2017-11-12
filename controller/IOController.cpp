#include "controller/IOController.h"
using namespace google;

IOController::IOController(Config &config) : 
	m_server(config.port(), config.certificate(), config.key()) {
	m_server.accept();
}

IOController::~IOController() {}

std::unique_ptr<protobuf::Message> IOController::getMessage() {
	return std::unique_ptr<protobuf::Message>();
}
