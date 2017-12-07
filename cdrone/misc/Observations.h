#ifndef __OBSERVATIONS_H__
#define __OBSERVATIONS_H__

#include <atomic>

// Observations holds observations made by different pieces of hardware. Each of
// the components should only be written to by a certain piece of hardware, but
// any controller can read from the object since each element is atomic. This
// object should be made as a shared_ptr and distributed amongst the objects.
class Observations {
public:
	Observations() : infraredHeight(0.0), infraredVoltage(0.0),
	cameraXMotion(0.0), cameraYMotion(0.0), cameraXPosition(0.0),
	cameraYPosition(0.0), cameraYawPosition(0.0), skylineBattery(0.0),
	skylineAccX(0.0), skylineAccY(0.0), skylineAccZ(0.0), skylineGyroX(0.0),
	skylineGyroY(0.0), skylineGyroZ(0.0), skylineAngRoll(0.0), skylineAngPitch(0.0),
	skylineAngYaw(0.0), skylineAngRollVel(0.0), skylineAngPitchVel(0.0),
	skylineAngYawVel(0.0), ioVelocityRoll(1500), ioVelocityPitch(1500),
	ioVelocityYaw(1500), ioVelocityThrottle(1000) {}

	// Infrared writes here
	std::atomic<double> infraredHeight;
	std::atomic<double> infraredVoltage;

	// Camera writes here
	std::atomic<double> cameraXMotion;
	std::atomic<double> cameraYMotion;
	std::atomic<double> cameraXPosition;
	std::atomic<double> cameraYPosition;
	std::atomic<double> cameraYawPosition;

	// Skyline writes here
	std::atomic<double> skylineBattery;
	std::atomic<double> skylineAccX;
	std::atomic<double> skylineAccY;
	std::atomic<double> skylineAccZ;
	std::atomic<double> skylineGyroX;
	std::atomic<double> skylineGyroY;
	std::atomic<double> skylineGyroZ;
	std::atomic<double> skylineAngRoll;
	std::atomic<double> skylineAngPitch;
	std::atomic<double> skylineAngYaw;
	std::atomic<double> skylineAngRollVel;
	std::atomic<double> skylineAngPitchVel;
	std::atomic<double> skylineAngYawVel;

	// IOController writes here
	std::atomic<uint16_t> ioVelocityRoll;
	std::atomic<uint16_t> ioVelocityPitch;
	std::atomic<uint16_t> ioVelocityYaw;
	std::atomic<uint16_t> ioVelocityThrottle;
};

#endif /* __OBSERVATIONS_H__ */
