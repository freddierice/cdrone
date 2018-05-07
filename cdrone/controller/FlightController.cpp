#include <stdexcept>

#include "controller/FlightController.h"
#include "logging/logging.h"
#include "logging/vector3.h"
#include <eigen3/Eigen/Geometry>


FlightController::FlightController(Config &config,
		std::shared_ptr<Observations> obs) : m_obs(obs), m_mode(Disarmed),
	m_lastModeChange(std::chrono::high_resolution_clock::now()),
	m_lastSendRC(std::chrono::high_resolution_clock::now()),
	m_skyline(config, obs), m_rollPID(config.rollPIDP(), config.rollPIDI(),
	config.rollPIDD(), 1500, 1300, 1700, 400), m_pitchPID(config.pitchPIDP(),
	config.pitchPIDI(), config.pitchPIDD(), 1500, 1300, 1700, 400),
	m_throttlePID(config.throttlePIDP(), config.throttlePIDI(),
			config.throttlePIDD(), config.throttlePIDK(), 1100, 1900, 400),
	m_posXPID(config.positionXPIDP(), config.positionXPIDI(),
			config.positionXPIDD(), config.positionXPIDCenter(), -0.2, 0.2, 400),
	m_posYPID(config.positionYPIDP(), config.positionYPIDI(),
			config.positionYPIDD(), config.positionXPIDCenter(), -0.2, 0.2, 400),
	m_posZPID(config.positionZPIDP(), config.positionZPIDI(),
			config.positionZPIDD(), config.positionZPIDCenter(), -0.1, 0.1, 400) {
}

FlightMode FlightController::getMode() {
	return m_mode.load(std::memory_order_acquire);
}

void FlightController::arm() {
	if (getMode() == Disarmed) {
		m_skyline.setArm();
		setMode(Armed);
		m_skyline.setIdle();
		m_throttlePID.reset();
	}
}

void FlightController::disarm() {
	auto mode = getMode();
	m_skyline.setDisarm();
	if (mode != Disarmed && mode != Disarming) {
		setMode(Disarmed);
	}
}

void FlightController::takeoff() {
	auto mode = getMode();
	if (mode == Armed)
		setMode(TakeOff);
}

void FlightController::rawControl() {
	auto mode = getMode();
	if (mode == VelocityControl || mode == PositionControl || mode == TakeOff)
		setMode(RawControl);
}

void FlightController::velocityControl() {
	auto mode = getMode();
	if (mode == RawControl || mode == PositionControl || mode == TakeOff) {
		resetPIDs();
		setMode(VelocityControl);
	}
}

void FlightController::positionControl() {
	auto mode = getMode();
	if (mode == RawControl || mode == VelocityControl || mode == TakeOff) {
		// m_obs->initialHeading = std::make_shared<Eigen::Quaterniond>(m_obs->quat0,
		// 		m_obs->quat1, m_obs->quat2, m_obs->quat3);
		m_obs->initialPositionX = static_cast<double>(m_obs->positionX);
		m_obs->initialPositionY = static_cast<double>(m_obs->positionY);
		m_obs->initialPositionZ = static_cast<double>(m_obs->positionZ);
		m_obs->initialYaw = static_cast<double>(m_obs->yaw);
		resetPIDs();
		setMode(PositionControl);
	}
}

void FlightController::resetPIDs() {
	m_rollPID.reset();
	m_pitchPID.reset();
	m_throttlePID.reset();
	m_posXPID.reset();
	m_posYPID.reset();
}

void FlightController::calibrate() {
	if (m_mode != Disarmed) {
		console->warn("could not calibrate, in mode {}", m_mode);
	}
	m_skyline.sendCalibrate();
	setMode(Calibrating);
}

void FlightController::updateRC() {
	m_skyline.update();
}

void FlightController::setTilt(uint16_t tilt) {
	m_skyline.setTilt(tilt);
}

logging::VariableLogger pos_logger("position", &logging::vector3_variable);
static inline void position(double &estimateX, double &estimateY, double &estimateZ, 
		std::shared_ptr<Observations> obs) {
	logging::vector3_t pos_log;

	estimateX = obs->positionX - obs->initialPositionX;
	estimateY = obs->positionY - obs->initialPositionY;
	estimateZ = obs->positionZ - obs->initialPositionZ;
	Eigen::Vector3d positionDiff(estimateX, estimateZ, estimateY);
	Eigen::Quaterniond quatDrone(obs->quat0, obs->quat1, obs->quat2, obs->quat3);
	positionDiff = quatDrone.toRotationMatrix() * positionDiff;
	estimateX = positionDiff[0];
	estimateY = positionDiff[2];
	estimateZ = positionDiff[1];

	pos_log.x = estimateX;
	pos_log.y = estimateY;
	pos_log.z = estimateZ;
	pos_logger.log(reinterpret_cast<void *>(&pos_log));
}

static inline void velocity(double &estimateX, double &estimateY, double &estimateZ, 
		std::shared_ptr<Observations> obs) {

	estimateX = obs->velocityX;
	estimateY = obs->velocityY;
	estimateZ = obs->velocityZ;
	Eigen::Vector3d velocityDiff(estimateX, estimateZ, estimateY);
	Eigen::Quaterniond quatDrone(obs->quat0, obs->quat1, obs->quat2, obs->quat3);
	velocityDiff = quatDrone.toRotationMatrix() * velocityDiff;
	estimateX = velocityDiff[0];
	estimateY = velocityDiff[2];
	estimateZ = velocityDiff[1];

	// pos_log.x = estimateX;
	// pos_log.y = estimateY;
	// pos_log.z = estimateZ;
	// pos_logger.log(reinterpret_cast<void *>(&pos_log));
}

void FlightController::updateController() {
	uint16_t roll, pitch, throttle;
	double velocityX, velocityY, velocityZ;
	double velocityTargetX, velocityTargetY, velocityTargetZ;
	double positionX, positionY, positionZ;
	std::chrono::high_resolution_clock::time_point then, now;
	FlightMode mode;

	mode = getMode();
	switch(mode) {
		case FlightMode::Disarmed:
			// don't do anything.. we are disarmed.
			break;
		case FlightMode::Disarming:
			// send a disarm signal to the multiwii board.
			m_skyline.setDisarm();
			// check if enough time has passed
			then = m_lastModeChange;
			disarm();
			break;
		case FlightMode::Calibrating:
			now = std::chrono::high_resolution_clock::now();
			then = m_lastModeChange;
			if (m_skyline.calibrateDone()) {
				console->warn("calibration done.");
				disarm();
				break;
			}
			if (now - then > std::chrono::seconds(10)) {
				console->warn("calibration taking too long. disarming.");
				disarm();
			}
			break;
		case FlightMode::TakeOff:
			velocityControl();
			break;
		case FlightMode::RawControl:
			m_skyline.setRC(m_obs->ioRawRoll, m_obs->ioRawPitch,
					m_obs->ioRawYaw, m_obs->ioRawThrottle);
			break;
		case FlightMode::VelocityControl:

			velocity(velocityX, velocityY, velocityZ, m_obs);
			position(positionX, positionY, positionZ, m_obs);
			
			velocityTargetZ = m_posZPID.step(positionZ);

			roll = (uint16_t)m_rollPID.step(m_obs->velocityX - m_obs->ioVelocityX);
			pitch = (uint16_t)m_pitchPID.step(velocityY - m_obs->ioVelocityY);
			throttle = (uint16_t)m_throttlePID.step(velocityZ - velocityTargetZ);

			m_skyline.setRC(roll, pitch, 1500, throttle);
			break;
		case FlightMode::PositionControl:

			velocity(velocityX, velocityY, velocityZ, m_obs);
			position(positionX, positionY, positionZ, m_obs);

			// use pos pid to get velocity targets.
			velocityTargetX = m_posXPID.step(positionX);
			velocityTargetY = m_posYPID.step(positionY);
			velocityTargetZ = m_posZPID.step(positionZ);

			// use velocity targets to reach angles (read: accelerations).
			roll = (uint16_t)m_rollPID.step(velocityX - velocityTargetX);
			pitch = (uint16_t)m_pitchPID.step(velocityY - velocityTargetY);
			throttle = (uint16_t)m_throttlePID.step(velocityZ - velocityTargetZ);

			m_skyline.setRC(roll, pitch, 1500, throttle);
			break;
		case FlightMode::Armed:
			break;
		case FlightMode::TouchDown:
			break;
		case FlightMode::Hover:
			break;
		default:
			//SPDLOG_DEBUG(spdlog::get("console"), "unhandled mode: {}", mode);
			throw std::runtime_error("got into a bad mode");
	}
}

void FlightController::setMode(FlightMode mode) {
	m_lastModeChange = std::chrono::high_resolution_clock::now();
	// SPDLOG_DEBUG(spdlog::get("console"), "MODE CHANGED FROM {} to {}", m_mode, mode);
	m_mode = mode;
}
