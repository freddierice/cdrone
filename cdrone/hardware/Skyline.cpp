#include "hardware/Skyline.h"

#include <string.h>

Skyline::Skyline(Serial &serial) : m_multiwii(serial) {}
Skyline::~Skyline() {}

void Skyline::sendArm() {
	m_multiwii.sendCMD(MultiWiiCMD::MSP_SET_RAW_RC, 
			(const void*)CMD_ARM, 19);
}

void Skyline::sendDisarm() {
	m_multiwii.sendCMD(MultiWiiCMD::MSP_SET_RAW_RC,
		(const void*)CMD_DISARM, 19);
}

void Skyline::sendCalibrate() {
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ACC_CALIBRATION);
}

void Skyline::update() {
	
}

// constants
const unsigned char Skyline::CMD_ARM[19] = {16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5, 228};
const unsigned char Skyline::CMD_DISARM[19] = {16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5, 216};
