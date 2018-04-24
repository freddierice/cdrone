#include <string.h>
#include <math.h>

#include "hardware/Skyline.h"
#include "logging/logging.h"
#include "logging/rc.h"
#include "logging/attitude.h"
#include "logging/vector3.h"
#include "misc/utility.h"

Skyline::Skyline(Config &config, std::shared_ptr<Observations> obs): m_serial(
		config.skylinePort()), m_multiwii(m_serial), m_obs(obs),
		m_calibrateFlag(false), m_attitudeFlag(false), m_imuFlag(false),
		m_analogFlag(false), m_armed(false), m_lastAttitudeTime(0.0),
		m_lastAnalog(std::chrono::high_resolution_clock::now()), m_ticks(0),
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

typedef struct RC_struct {
	uint16_t roll;
	uint16_t pitch;
	uint16_t yaw;
	uint16_t thrust;
	uint16_t aux1;
} RC_t;

logging::VariableLogger rc_logger("rc", &logging::rc_variable);
void Skyline::sendRC() {
	RC_t rc;
	logging::rc_t rc_log;
	
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

	rc_log.roll = rc.roll;
	rc_log.pitch = rc.pitch;
	rc_log.yaw = rc.yaw;
	rc_log.thrust = rc.thrust;
	rc_log.aux1 = rc.aux1;
	
	m_multiwii.sendCMD(MultiWiiCMD::MSP_SET_RAW_RC, (char *)&rc, 10);
	rc_logger.log((void *)&rc_log);
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

logging::VariableLogger attitude_logger("attitude", &logging::attitude_variable);
logging::VariableLogger acc_logger("acc", &logging::vector3_variable);
logging::VariableLogger gyro_logger("gyro", &logging::vector3_variable);
void Skyline::update() {
	MultiWiiResponse resp;
	MSP_ATTITUDE_T *attitude;
	MSP_ANALOG_T *analog;
	MSP_IMU_T *imu;
	double lastRoll, lastPitch, lastYaw;
	double now, dt;

	// ask for battery every 3 seconds.
	auto now_time = std::chrono::high_resolution_clock::now();
	if (now_time - m_lastAnalog > std::chrono::seconds(3)) {
		m_lastAnalog = now_time;
		sendAnalog();
	}

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

				// calculate the variables and log them
				logging::attitude_t att;
				att.roll = (((double)attitude->angx) + 1800.0) / 1800.0 * M_PI;
				att.pitch = (((double)attitude->angy) + 900.0) / 900.0 * M_PI;
				att.yaw = ((double)attitude->heading) / 180.0 * M_PI;
				attitude_logger.log(&att);

				// update the now vars
				m_obs->skylineAngRoll = att.roll;
				m_obs->skylineAngPitch = att.pitch;
				m_obs->skylineAngYaw = att.yaw;

				// get new angle diffs
				m_obs->skylineDAngRoll = atan2(sin(att.roll - lastRoll), cos(att.roll - lastRoll));
				m_obs->skylineDAngPitch = atan2(sin(att.pitch - lastPitch), cos(att.pitch - lastPitch));
				m_obs->skylineDAngYaw = atan2(sin(att.yaw - lastYaw), cos(att.yaw - lastYaw));

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
				break;
			case MSP_RAW_IMU:
				// XXX: using TA values... figure out a better way.
				imu = (MSP_IMU_T *)resp.m_data;
				logging::vector3_t acc_log, gyro_log;
				
				acc_log.x = (imu->accx-ACC_X_ZERO)*ACC_RAW_TO_MSS;
				acc_log.y = (imu->accy-ACC_Y_ZERO)*ACC_RAW_TO_MSS;
				acc_log.z = (imu->accz-ACC_Z_ZERO)*ACC_RAW_TO_MSS;

				gyro_log.x = (double)imu->gyrx * GYRO_RAW_TO_RAD;
				gyro_log.y = imu->gyry * GYRO_RAW_TO_RAD;
				gyro_log.z = imu->gyrz * GYRO_RAW_TO_RAD;

				m_obs->skylineAccX = acc_log.x;
				m_obs->skylineAccY = acc_log.y;
				m_obs->skylineAccZ = acc_log.z;
				m_obs->skylineGyroX = gyro_log.x;
				m_obs->skylineGyroY = gyro_log.y;
				m_obs->skylineGyroZ = gyro_log.z;

				// m_imuFlag = false;
				sendIMU();
				acc_logger.log(&acc_log);
				gyro_logger.log(&gyro_log);
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
const double Skyline::GYRO_RAW_TO_RAD = 0.004261057744004991;
const double Skyline::ACC_RAW_TO_MSS = 0.019117690177075743;
const double Skyline::ACC_X_ZERO = 0.3336673346693387;
const double Skyline::ACC_Y_ZERO = 0.22745490981963928;
const double Skyline::ACC_Z_ZERO = 512.6142284569138;
