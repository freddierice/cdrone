#ifndef __OBSERVATIONS_H__
#define __OBSERVATIONS_H__

#include <atomic>

// Observations holds observations made by different pieces of hardware. Each of
// the components should only be written to by a certain piece of hardware, but
// any controller can read from the object since each element is atomic. This
// object should be made as a shared_ptr and distributed amongst the objects.
class Observations {
public:
	Observations() : velocityX(0.0), velocityY(0.0), velocityZ(0.0),
	velocityYaw(0.0), positionX(0.0), positionY(0.0), positionZ(0.0),
	yaw(0.0), initialPositionX(0.0), initialPositionY(0.0),
	initialPositionZ(0.0), initialYaw(0.0), skylineBattery(0.0),
	skylineAccX(0.0), skylineAccY(0.0), skylineAccZ(0.0), skylineGyroX(0.0),
	skylineGyroY(0.0), skylineGyroZ(0.0), skylineAngRoll(0.0),
	skylineAngPitch(0.0), skylineAngYaw(0.0), skylineDAngRoll(0.0),
	skylineDAngPitch(0.0), skylineDAngYaw(0.0), skylineAngRollVel(0.0),
	skylineAngPitchVel(0.0), skylineAngYawVel(0.0), ioRawRoll(1500),
	ioRawPitch(1500), ioRawYaw(1500), ioRawThrottle(1000), ioVelocityX(0.0),
	ioVelocityY(0.0), ioPositionX(0.0), ioPositionY(0.0), ioPositionZ(0.35) {}

	std::atomic<double> velocityX;
	std::atomic<double> velocityY;
	std::atomic<double> velocityZ;
	std::atomic<double> velocityYaw;
	std::atomic<double> positionX;
	std::atomic<double> positionY;
	std::atomic<double> positionZ;
	std::atomic<double> yaw;
	std::atomic<double> initialPositionX;
	std::atomic<double> initialPositionY;
	std::atomic<double> initialPositionZ;
	std::atomic<double> initialYaw;
	
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
	std::atomic<double> skylineDAngRoll;
	std::atomic<double> skylineDAngPitch;
	std::atomic<double> skylineDAngYaw;
	std::atomic<double> skylineAngRollVel;
	std::atomic<double> skylineAngPitchVel;
	std::atomic<double> skylineAngYawVel;

	// IOController writes here
	std::atomic<uint16_t> ioRawRoll;
	std::atomic<uint16_t> ioRawPitch;
	std::atomic<uint16_t> ioRawYaw;
	std::atomic<uint16_t> ioRawThrottle;
	std::atomic<double> ioVelocityX;
	std::atomic<double> ioVelocityY;
	std::atomic<double> ioPositionX;
	std::atomic<double> ioPositionY;
	std::atomic<double> ioPositionZ;

	void resetIO() { // reset the io values to their defaults.
		ioRawRoll = 1500;
		ioRawPitch = 1500;
		ioRawYaw = 1500;
		ioRawThrottle = 1000;
		ioVelocityX = 0;
		ioVelocityY = 0;
		ioPositionX = 0;
		ioPositionY = 0;
		ioPositionZ = 0.35;
	};
};

#endif /* __OBSERVATIONS_H__ */
