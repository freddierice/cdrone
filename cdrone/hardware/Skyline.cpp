#include <string.h>
#include <math.h>

#include "hardware/Skyline.h"
#include "misc/logging.h"
#include "misc/utility.h"

Skyline::Skyline(Config &config, std::shared_ptr<Observations> obs): m_serial(
		config.skylinePort()), m_multiwii(m_serial), m_obs(obs) {}
Skyline::~Skyline() {}

void Skyline::sendArm() {
	sendRC(1500, 1500, 2000, 1000);
}

void Skyline::sendDisarm() {
	// spec actually says to use 1000 as the throttle, but the skyline will
	// accept a lower throttle. It should be used for safety. Or not. 990 until
	// changed to something better.
	sendRC(1500, 1500, 1000, 990);
}

void Skyline::sendIdle() {
	sendRC(1500, 1500, 1500, 1000);
}

void Skyline::sendRC(uint16_t roll, uint16_t pitch, uint16_t yaw,
		uint16_t throttle) {
	char cmd[16];
	uint16_t *cmdInt = (uint16_t *)&cmd;
	
#if defined(__BYTE_ORDER__) &&__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	*cmdInt++ = roll;
	*cmdInt++ = pitch;
	*cmdInt++ = yaw;
	*cmdInt++ = throttle;
	*cmdInt++ = 1500;
	*cmdInt++ = 1500;
	*cmdInt++ = 1500;
	*cmdInt   = 1500;
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	#error "Not Yet Implemented"
#else
	#error "Compiler is not compatible -- use GCC"
#endif
	
	m_multiwii.sendCMD(MultiWiiCMD::MSP_SET_RAW_RC, cmd, 16);
}

void Skyline::sendCalibrate() {
	m_calibrateFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ACC_CALIBRATION);
}
bool Skyline::calibrateDone() {
	return !m_calibrateFlag;
}
void Skyline::sendAttitude() {
	m_attitudeFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ATTITUDE);
}
bool Skyline::attitudeDone() {
	return !m_attitudeFlag;
}
void Skyline::sendIMU() {
	m_imuFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_RAW_IMU);
}
bool Skyline::imuDone() {
	return !m_imuFlag;
}
void Skyline::sendAnalog() {
	m_analogFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ANALOG);
}
bool Skyline::analogDone() {
	return !m_analogFlag;
}

void Skyline::update() {
	MultiWiiResponse resp;
	MSP_ATTITUDE_T *attitude;
	MSP_ANALOG_T *analog;
	MSP_IMU_T *imu;
	double lastRoll, lastPitch, lastYaw;
	double now, dt;

	// try to get the readings every update.
	sendAttitude();

	// field at most 10 reads at a time
	for (int i = 0; i < 10; i++) {
		// get messages ready from multiwii.
		if (!m_multiwii.recv(resp))
			return;

		// process it.
		switch (resp.m_type) {
			case MSP_ACC_CALIBRATION:
				m_calibrateFlag = false;
				break;
			case MSP_ATTITUDE:
				attitude = (MSP_ATTITUDE_T *)resp.m_data;
				lastRoll = m_obs->skylineAngRoll;
				lastPitch = m_obs->skylineAngPitch;
				lastYaw = m_obs->skylineAngYaw;
				m_obs->skylineAngRoll = (double)attitude->angx/180.0 * M_PI;
				m_obs->skylineAngPitch = (double)attitude->angy/180.0 * M_PI;
				m_obs->skylineAngYaw = (double)attitude->heading/180.0 * M_PI;
				now = ::get_time();
				dt = now - m_lastAttitudeTime;
				m_lastAttitudeTime = now;
				m_obs->skylineAngRollVel = (m_obs->skylineAngRoll - lastRoll)/dt;
				m_obs->skylineAngPitchVel = (m_obs->skylineAngPitch - lastPitch)/dt;
				m_obs->skylineAngYawVel = (m_obs->skylineAngYaw - lastYaw)/dt;
				m_attitudeFlag = false;
				break;
			case MSP_ANALOG:
				analog = (MSP_ANALOG_T *)resp.m_data;
				m_obs->skylineBattery = (double)analog->vbat/10.0;
				m_analogFlag = false;
				break;
			case MSP_RAW_IMU:
				// XXX: using TA values... figure out a better way.
				imu = (MSP_IMU_T *)resp.m_data;
				m_obs->skylineAccX = (imu->accx-ACC_X_ZERO)*ACC_RAW_TO_MSS;
				m_obs->skylineAccY = (imu->accy-ACC_Y_ZERO)*ACC_RAW_TO_MSS;
				m_obs->skylineAccZ = (imu->accz-ACC_Z_ZERO)*ACC_RAW_TO_MSS;
				m_obs->skylineGyroX = imu->gyrx;
				m_obs->skylineGyroY = imu->gyry;
				m_obs->skylineGyroZ = imu->gyrz;
				m_imuFlag = false;
				break;
			// ignore these
			case MSP_STATUS:
			case MSP_BOXIDS:
			case MSP_SET_BOX:
			case MSP_BOX:
			case MSP_IDENT:
			case MSP_RC:
			case MSP_ALTITUDE:
			case MSP_MOTOR:
				break;
			default:
				console->warn("skyline got unexpected m_type: {}", (int)resp.m_type);
				break;
		}
	}
}

// constants
const unsigned char Skyline::CMD_ARM[18] = {16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5};
const unsigned char Skyline::CMD_DISARM[18] = {16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5};
const double Skyline::ACC_RAW_TO_MSS = 0.019117690177075743;
const double Skyline::ACC_X_ZERO = 0.3336673346693387;
const double Skyline::ACC_Y_ZERO = 0.22745490981963928;
const double Skyline::ACC_Z_ZERO = 512.6142284569138;
