#ifndef __MULTI_WII_H__
#define __MULTI_WII_H__

#include <inttypes.h>

#include "wire/Serial.h"

enum MultiWiiCMD {
	// 100s getting
	MSP_IDENT = 100,
	MSP_STATUS = 101,
	MSP_RAW_IMU = 102,
	MSP_SERVO = 103,
	MSP_MOTOR = 104,
	MSP_RC = 105,
	MSP_RAW_GPS = 106,
	MSP_COMP_GPS = 107,
	MSP_ATTITUDE = 108,
	MSP_ALTITUDE = 109,
	MSP_ANALOG = 110,
	MSP_RC_TUNING = 111,
	MSP_PID = 112,
	MSP_BOX = 113,
	MSP_MISC = 114,
	MSP_MOTOR_PINS = 115,
	MSP_BOXNAMES = 116,
	MSP_PIDNAMES = 117,
	MSP_WP = 118,
	MSP_BOXIDS = 119,
	MSP_SERVO_CONF = 120,

	// 200s setting
	MSP_SET_RAW_RC = 200,
	MSP_SET_RAW_GPS = 201,
	MSP_SET_PID = 202,
	MSP_SET_BOX = 203,
	MSP_SET_RC_TUNING = 204,
	MSP_ACC_CALIBRATION = 205,
	MSP_MAG_CALIBRATION = 206,
	MSP_SET_MISC = 207,
	MSP_RESET_CONF = 208,
	MSP_SET_WP = 209,
	MSP_SELECT_SETTING = 210,
	MSP_SET_HEAD = 211,
	MSP_SET_SERVO_CONF = 212,
	// 213 does not exist.
	MSP_SET_MOTOR = 214,
	MSP_BIND = 240,
	MSP_EEPROM_WRITE = 250
};

class MultiWiiResponse {
public:
	MultiWiiCMD m_type;
	char m_data[256];
	int m_dataLen;
};

class MultiWii {
public:
	MultiWii(Serial& serial);
	~MultiWii();

	void sendCMD(MultiWiiCMD cmd);
	void sendCMD(MultiWiiCMD cmd, const void* buf, uint8_t n);
	void sendRaw(const void* buffer, int n);

	bool recv(MultiWiiResponse& response);
	
private:
	Serial& m_serial;
	char* m_send;
	char* m_recv;
	int m_idx;
};

// structs for handling different types of data

// MSP_SET_RAW_RC
typedef struct MSP_RC_struct {
	uint16_t roll;
	uint16_t pitch;
	uint16_t yaw;
	uint16_t throttle;
} MSP_RC_T;

// MSP_ATTITUDE
typedef struct MSP_ATTITUDE_struct {
	int16_t angx; // [-1800, 1800]
	int16_t angy; // [-900, 900]
	uint16_t heading; // [-180, 180]
} MSP_ATTITUDE_T;

// MSP_ANALOG
typedef struct MSP_ANALOG_struct {
	uint8_t vbat;
	uint16_t intPowerMeterSum;
	uint16_t rssi;
	uint16_t amperage;
} MSP_ANALOG_T;

// MSP_RAW_IMU
typedef struct MSP_IMU_struct {
	uint16_t accx;
	uint16_t accy;
	uint16_t accz;
	uint16_t gyrx;
	uint16_t gyry;
	uint16_t gyrz;
	uint16_t magx;
	uint16_t magy;
	uint16_t magz;
} MSP_IMU_T;

#endif /* __MULTI_WII_H__ */
