#ifndef __MULTIWII_H__
#define __MULTIWII_H__

#include <inttypes.h>

class Serial;

enum MultiWiiCMD {
	IDENT = 100,
	MSP_STATUS = 101,
	MSP_RAW_IMU = 102,
	MSP_RC = 105,
	MSP_SET_RAW_RC = 200,
	MSP_ATTITUDE = 108,
	MSP_ACC_CALIBRATION = 205,
	MSP_MAG_CALIBRATION = 206
};

class MultiWii {
public:
	MultiWii(Serial& serial);
	~MultiWii();

	bool sendCMD(MultiWiiCMD cmd);
	bool sendCMD(MultiWiiCMD cmd, char *buf, uint8_t n);
	

	bool sendArm();
	bool sendDisarm();
	
	// prebuild commands
	static const unsigned char CMD_ARM[22];
	static const unsigned char CMD_DISARM[22];
private:
	Serial& m_serial;
	char* m_send;
	char* m_recv;
};

#endif /* __MULTIWII_H__ */
