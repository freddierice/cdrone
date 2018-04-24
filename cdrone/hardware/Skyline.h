#ifndef __SKYLINE_H__
#define __SKYLINE_H__

#include <chrono>
#include <memory>

#include <inttypes.h>

#include "misc/Config.h"
#include "misc/Observations.h"
#include "wire/MultiWii.h"
#include "wire/Serial.h"

class Skyline {
public:
	// create a skyline with a serial interface.
	Skyline(Config &config, std::shared_ptr<Observations> obs);
	~Skyline();

	// start sends a single sequence of msp commands to the flight controller
	// to start the call and response from the controller.
	void start();

	// setArm sends a single arm command to the skyline.
	void setArm();
	
	// sendDisarm sends a single disarm command to the skyline.
	void setDisarm();

	// setIdle sends an idle command to the skyline.
	void setIdle();

	// setRC sets the rc values to get sent to the skyline.
	void setRC(uint16_t roll, uint16_t pitch, uint16_t yaw, uint16_t throttle);

	// sendCalibrate sends a single calibrate command to the skyline.
	void sendCalibrate();
	bool calibrateDone();

	// update recvs and reinitiates sends on the MSP controller.
	void update();

	static const unsigned char CMD_ARM[];
	static const unsigned char CMD_DISARM[];
	static const double ACC_RAW_TO_MSS;
	static const double GYRO_RAW_TO_RAD;
	static const double ACC_X_ZERO;
	static const double ACC_Y_ZERO;
	static const double ACC_Z_ZERO;
private:
	// default constructor not allowed.
	Skyline() = delete;

	// sendRC sends roll, pitch, yaw, and throttle to the skyline. The values
	// must lie between 1000 and 2000.
	void sendRC();

	// sendAttitude sends a single attitude request to the skyline.
	void sendAttitude();

	// sendIMU sends a single IMU request to the skyline.
	void sendIMU();

	// sendAnalog sends a single analog request to the skyline.
	void sendAnalog();

	// update reads responses from the wire and sets flags.
	// a multiwii protocol parser for understanding messages that go from the
	// skyline to me.
	Serial m_serial;
	MultiWii m_multiwii;

	// world observations to record.
	std::shared_ptr<Observations> m_obs;

	// flag is true while waiting for calibration response.
	std::atomic<bool> m_calibrateFlag;
	std::atomic<bool> m_attitudeFlag;
	std::atomic<bool> m_imuFlag;
	std::atomic<bool> m_analogFlag;
	std::atomic<bool> m_armed;

	// time
	double m_lastAttitudeTime;
	std::chrono::high_resolution_clock::time_point m_lastAnalog;

	// ticks
	int m_ticks;
	
	// rc channels
	std::atomic<uint16_t> m_roll;
	std::atomic<uint16_t> m_pitch;
	std::atomic<uint16_t> m_yaw;
	std::atomic<uint16_t> m_throttle;
};

#endif /* __SKYLINE_H__ */
