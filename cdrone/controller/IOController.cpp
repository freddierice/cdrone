#include "controller/IOController.h"

IOController::IOController(Config &config) : 
	m_server(config), m_bufferLen(4096), m_connected(false) {
	m_buffer = new char[m_bufferLen]();
}

IOController::~IOController() {}

void IOController::accept() {
	auto zeroCopyStreams = m_server.accept();
	m_input = std::make_unique<google::protobuf::io::CodedInputStream>(zeroCopyStreams.first);
	m_output = std::make_unique<google::protobuf::io::CodedOutputStream>(zeroCopyStreams.second);
	m_connected = true;
}

bool IOController::connected() {
	return m_connected;
}

void IOController::getMessage(google::protobuf::Message &message) {
	// message.ParseFromZeroCopyStream(m_input);

	uint64_t n;
	if (!m_input->ReadVarint64(&n)) 
		throw IOControllerException("could not read in ReadVarint64");

	// try to parse directly from the buffer
	const void* data;
	int size;
	if (!m_input->GetDirectBufferPointer(&data, &size))
		throw IOControllerException("could not read in GetDirectBufferPointer");
	if ((uint64_t)size >= n) {
		if (!message.ParseFromArray(data, n))
			throw IOControllerException("could not read in ParseFromArray");
		m_input->Skip(n);
		return;
	}

	// otherwise, read in less efficiently: 
	if (m_bufferLen < n) {
		delete m_buffer;
		m_bufferLen = ((n/m_bufferLen)+1)*m_bufferLen;
		m_buffer = new char[m_bufferLen]();
	}

	if (!m_input->ReadRaw(m_buffer, n))
		throw IOControllerException("could not read in ReadRaw");
	if (!message.ParseFromArray(m_buffer, n)) 
		throw IOControllerException("could not read in ParseFromArray");
}
