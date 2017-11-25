#include "wire/MultiWii.h"

#include <string.h>


MultiWii::MultiWii(Serial &serial) : m_serial(serial), 
		m_send(new char[256]()), m_recv(new char[256]()), m_idx(0) {
	// initialize with preamble $M<
	m_send[0] = 36; m_send[1] = 77; m_send[2] = 60;
}

MultiWii::~MultiWii() {
	delete m_send;
	delete m_recv;
}

void MultiWii::sendCMD(MultiWiiCMD cmd) {
	sendCMD(cmd, NULL, 0);
}

void MultiWii::sendCMD(MultiWiiCMD cmd, const void* buf, uint8_t n) {
	m_send[3] = (char)n;
	m_send[4] = (char)cmd;
	if (n) ::memcpy(m_send+5, buf, n);
	m_send[n+5] = 0;
	for (int i = 3; i < n+5; i++) {
		m_send[n+5] ^= m_send[i];
	}
	return m_serial.writeFull(m_send, n+6);
}

void MultiWii::sendRaw(const void *buffer, int n) {
	m_serial.writeFull(buffer, n);
}

bool MultiWii::recv(MultiWiiResponse &response) {
	// first read into the buffer what we want.
	if (m_idx < 4) {
		m_idx += m_serial.read(m_recv+m_idx, 4 - m_idx);
		if (m_idx < 4)
			return false;
	}
	int len = (int)m_recv[3]+5;
	if (m_idx != len) {
		m_idx += m_serial.read(m_recv+m_idx, len - m_idx);
		if (m_idx != len)
			return false;
	}

	// copy into the response and return true
	response.m_type = static_cast<MultiWiiCMD>(m_recv[4]);
	response.m_dataLen = len-5;
	memcpy(response.m_data, m_recv, len);
	m_idx = 0;

	return true;
}
