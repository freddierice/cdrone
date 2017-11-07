#include "controller/MultiWii.h"
#include "wire/Serial.h"

#include <string.h>


MultiWii::MultiWii(Serial& serial) : m_serial(serial) {
	// initialize with preamble $M<
	this->m_send = new char[256]();
	this->m_send[0] = 36; this->m_send[1] = 77; this->m_send[2] = 60;
	this->m_recv = new char[256]();
}

MultiWii::~MultiWii() {
	delete this->m_send;
	delete this->m_recv;
}

bool MultiWii::sendCMD(MultiWiiCMD cmd) {
	return sendCMD(cmd, NULL, 0);
}

bool MultiWii::sendCMD(MultiWiiCMD cmd, char *buf, uint8_t n) {
	this->m_send[3] = (char)n;
	this->m_send[4] = (char)cmd;
	if (n) ::memcpy(this->m_send+5, buf, n);
	for (int i = 3; i < n+5; i++) {
		this->m_send[n+5] ^= this->m_send[i];
	}
	return this->m_serial.writeFull(this->m_send, n+6);
}

bool MultiWii::sendArm() {
	return this->m_serial.writeFull((const char *)CMD_ARM, sizeof(CMD_ARM));
}

bool MultiWii::sendDisarm() {
	return this->m_serial.writeFull((const char *)CMD_DISARM, sizeof(CMD_DISARM));
}

// constants
const unsigned char MultiWii::CMD_ARM[22] = {36, 77, 60, 16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5, 228};
const unsigned char MultiWii::CMD_DISARM[22] = {36, 77, 60, 16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5, 216};
