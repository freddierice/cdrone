#ifndef __SKYLINE_H__
#define __SKYLINE_H__

#include <inttypes.h>

#include "wire/MultiWii.h"
#include "wire/Serial.h"

class Skyline {
public:
	Skyline(Serial &serial);
	~Skyline();

	bool sendArm();
	bool sendDisarm();
	bool calibrate();
	
	static const unsigned char CMD_ARM[];
	static const unsigned char CMD_DISARM[];
private:
	MultiWii m_multiwii;
};

#endif /* __SKYLINE_H__ */
