#ifndef __MULTI_WII_H__
#define __MULTI_WII_H__

#include <inttypes.h>

#include "wire/Serial.h"

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
	MultiWii(Serial &serial);
	~MultiWii();

	bool sendCMD(MultiWiiCMD cmd);
	bool sendCMD(MultiWiiCMD cmd, const void* buf, uint8_t n);
	bool sendRaw(const void* buffer, int n);
	
private:
	Serial& m_serial;
	char* m_send;
	char* m_recv;
};

#endif /* __MULTI_WII_H__ */
