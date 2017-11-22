#ifndef __SKYLINE_H__
#define __SKYLINE_H__

#include <inttypes.h>

#include "wire/MultiWii.h"
#include "wire/Serial.h"

class Skyline {
public:
	// create a skyline with a serial interface.
	Skyline(Serial &serial);
	~Skyline();

	// sendArm sends a single arm command to the skyline.
	void sendArm();

	// sendCalibrate sends a single calibrate command to the skyline.
	void sendCalibrate();

	// sendDisarm sends a single disarm command to the skyline.
	void sendDisarm();

	// update reads responses from the wire and sets flags.
	void update();
	
	static const unsigned char CMD_ARM[];
	static const unsigned char CMD_DISARM[];
private:
	// a multiwii protocol parser for understanding messages that go from the
	// skyline to me.
	MultiWii m_multiwii;
};

#endif /* __SKYLINE_H__ */
