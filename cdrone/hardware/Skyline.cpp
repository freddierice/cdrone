#include <string.h>
#include <math.h>

#include "hardware/Skyline.h"
#include "logging/logging.h"
#include "logging/rc.h"
#include "misc/utility.h"

Skyline::Skyline(Config &config, std::shared_ptr<Observations> obs): m_serial(
		config.skylinePort()), m_multiwii(m_serial), m_obs(obs),
		m_calibrateFlag(false), m_attitudeFlag(false), m_imuFlag(false),
		m_analogFlag(false), m_armed(false), m_lastAttitudeTime(0.0), m_ticks(0),
		m_roll(1500), m_pitch(1500), m_yaw(1500), m_throttle(1000) {
	setIdle();
	start();
}
Skyline::~Skyline() {}

void Skyline::start() {
	sendRC();
	sendAnalog();
	sendIMU();
	sendAttitude();
}

void Skyline::setArm() {
	m_armed = true;
	setRC(1500, 1500, 1500, 900);
}

void Skyline::setDisarm() {
	// spec actually says to use 1000 as the throttle, but the skyline will
	// accept a lower throttle. It should be used for safety. Or not. 990 until
	// changed to something better.
	m_armed = false;
	setRC(1500, 1500, 1500, 900);
}

void Skyline::setIdle() {
	setRC(1500, 1500, 1500, 900);
}

void Skyline::setRC(uint16_t roll, uint16_t pitch, uint16_t yaw,
		uint16_t throttle) {
	m_roll = roll;
	m_pitch = pitch;
	m_yaw = yaw;
	m_throttle = throttle;
}

logging::VariableLogger rc_logger("rc", &logging::rc_variable);
void Skyline::sendRC() {
	logging::rc_t rc;
	
#if defined(__BYTE_ORDER__) &&__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	rc.roll = m_roll;
	rc.pitch = m_pitch;
	rc.yaw = m_yaw;
	rc.thrust = m_throttle;
	rc.aux1 = m_armed ? 1800 : 1000; // aux 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	#error "Not Yet Implemented"
#else
	#error "Compiler is not compatible -- use GCC"
#endif
	
	m_multiwii.sendCMD(MultiWiiCMD::MSP_SET_RAW_RC, (char *)&rc, 10);
	rc_logger.log((void *)&rc);
}

void Skyline::sendCalibrate() {
	m_calibrateFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ACC_CALIBRATION);
}
bool Skyline::calibrateDone() {
	return !m_calibrateFlag;
}
void Skyline::sendAttitude() {
	// m_attitudeFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ATTITUDE);
}
void Skyline::sendIMU() {
	// m_imuFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_RAW_IMU);
}
void Skyline::sendAnalog() {
	// m_analogFlag = true;
	m_multiwii.sendCMD(MultiWiiCMD::MSP_ANALOG);
}

void Skyline::update() {
	MultiWiiResponse resp;
	MSP_ATTITUDE_T *attitude;
	MSP_ANALOG_T *analog;
	MSP_IMU_T *imu;
	double lastRoll, lastPitch, lastYaw;
	double now, dt;

	// process it. (expect to process one of each)
	for (int i = 0; i < 4; i++) {
		// get messages ready from multiwii.
		if (!m_multiwii.recv(resp))
			return;
	
		switch (resp.m_type) {
			case MSP_ACC_CALIBRATION:
				m_calibrateFlag = false;
				break;
			case MSP_ATTITUDE:
				attitude = (MSP_ATTITUDE_T *)resp.m_data;
				// update time
				now = ::get_time();
				dt = now - m_lastAttitudeTime;
				m_lastAttitudeTime = now;
				
				// update the last vars
				lastRoll = m_obs->skylineAngRoll;
				lastPitch = m_obs->skylineAngPitch;
				lastYaw = m_obs->skylineAngYaw;

				// update the now vars
				m_obs->skylineAngRoll = (((double)attitude->angx) + 1800.0) / 1800.0 * M_PI;
				m_obs->skylineAngPitch = (((double)attitude->angy) + 900.0) / 900.0 * M_PI; ;
				m_obs->skylineAngYaw = ((double)attitude->heading) / 180.0 * M_PI;

				// get new angle diffs
				m_obs->skylineDAngRoll = atan2(sin(m_obs->skylineAngRoll - lastRoll), cos(m_obs->skylineAngRoll - lastRoll));
				m_obs->skylineDAngPitch = atan2(sin(m_obs->skylineAngPitch - lastPitch), cos(m_obs->skylineAngPitch - lastPitch));
				m_obs->skylineDAngYaw = atan2(sin(m_obs->skylineAngYaw - lastYaw), cos(m_obs->skylineAngYaw - lastYaw));

				m_obs->skylineAngRollVel = m_obs->skylineDAngRoll/dt;
				m_obs->skylineAngPitchVel = m_obs->skylineDAngPitch/dt;
				m_obs->skylineAngYawVel = m_obs->skylineDAngYaw/dt;

				// m_attitudeFlag = false;
				sendAttitude();
				break;
			case MSP_ANALOG:
				analog = (MSP_ANALOG_T *)resp.m_data;
				m_obs->skylineBattery = (double)analog->vbat/10.0;
				// m_analogFlag = false;
				sendAnalog();
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
				// m_imuFlag = false;
				sendIMU();
				break;
			case MSP_SET_RAW_RC:
				sendRC();
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
	// m_serial.flush();
}

// constants
const unsigned char Skyline::CMD_ARM[18] = {16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5};
const unsigned char Skyline::CMD_DISARM[18] = {16, 200, 220, 5, 220, 5, 208, 7, 232, 3, 220, 5, 220, 5, 220, 5, 220, 5};
const double Skyline::ACC_RAW_TO_MSS = 0.019117690177075743;
const double Skyline::ACC_X_ZERO = 0.3336673346693387;
const double Skyline::ACC_Y_ZERO = 0.22745490981963928;
const double Skyline::ACC_Z_ZERO = 512.6142284569138;
