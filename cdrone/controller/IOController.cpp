#include "controller/IOController.h"

IOController::IOController(Config &config) : 
	m_server(config) {
	auto zeroCopyStreams = m_server.accept();
	m_input = zeroCopyStreams.first;
	m_output = zeroCopyStreams.second;
}

IOController::~IOController() {}

void IOController::getMessage(google::protobuf::Message &message) {
	message.ParseFromZeroCopyStream(m_input);
}
