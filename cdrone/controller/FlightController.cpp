#include <stdexcept>

#include "controller/FlightController.h"
#include "logging/logging.h"


FlightController::FlightController(Config &config,
		std::shared_ptr<Observations> obs) : m_obs(obs), m_mode(Disarmed),
	m_lastModeChange(std::chrono::high_resolution_clock::now()),
	m_lastSendRC(std::chrono::high_resolution_clock::now()),
	m_skyline(config, obs), m_rollPID(config.rollPIDP(), config.rollPIDI(),
	config.rollPIDD(), 1500, 1400, 1600), m_pitchPID(config.pitchPIDP(),
	config.pitchPIDI(), config.pitchPIDD(), 1500, 1400, 1600),
	m_throttlePID(config.throttlePIDP(), config.throttlePIDI(),
			config.throttlePIDD(), config.throttlePIDK(), 1100, 1900), 
	m_posXPID(-0.04, -0.001, 0, 0, -0.20, 0.20), m_posYPID(-0.04, -0.001, 0, 0, -0.20, 0.20) {
}

FlightMode FlightController::getMode() {
	return m_mode.load(std::memory_order_acquire);
}

void FlightController::arm() {
	if (getMode() == Disarmed) {
		m_skyline.setArm();
		setMode(Armed);
		m_skyline.setIdle();
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
	if (mode == VelocityControl || mode == PositionControl)
		setMode(RawControl);
}

void FlightController::velocityControl() {
	auto mode = getMode();
	if (mode == RawControl || mode == PositionControl) {
		m_rollPID.reset();
		m_pitchPID.reset();
		m_throttlePID.reset();
		setMode(VelocityControl);
	}
}

void FlightController::positionControl() {
	auto mode = getMode();
	if (mode == RawControl || mode == VelocityControl) {
		m_obs->initialPositionX = static_cast<double>(m_obs->positionX);
		m_obs->initialPositionY = static_cast<double>(m_obs->positionY);
		m_obs->initialPositionZ = static_cast<double>(m_obs->positionZ);
		m_obs->initialYaw = static_cast<double>(m_obs->yaw);
		setMode(PositionControl);
	}
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

void FlightController::updateController() {
	uint16_t roll, pitch, throttle;
	double estimateX, estimateY, estimateZ, estimateYaw;
	double vx, vy;
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
			setMode(FlightMode::Disarmed);
			break;
		case FlightMode::Calibrating:
			now = std::chrono::high_resolution_clock::now();
			then = m_lastModeChange;
			if (m_skyline.calibrateDone()) {
				setMode(FlightMode::Disarmed);
				break;
			}
			if (now - then > std::chrono::seconds(10)) {
				console->warn("calibration taking too long. disarming.");
				setMode(FlightMode::Disarmed);
			}
			break;
		case FlightMode::TakeOff:
			setMode(VelocityControl);
			break;
		case FlightMode::RawControl:
			m_skyline.setRC(m_obs->ioRawRoll, m_obs->ioRawPitch,
					m_obs->ioRawYaw, m_obs->ioRawThrottle);
			break;
		case FlightMode::VelocityControl:
			estimateZ = m_obs->initialPositionZ + m_obs->ioPositionZ;
			estimateZ -= m_obs->positionZ;
			roll = (uint16_t)m_rollPID.step(m_obs->velocityX - m_obs->ioVelocityX);
			pitch = (uint16_t)m_pitchPID.step(m_obs->velocityY - m_obs->ioVelocityY);
			throttle = (uint16_t)m_throttlePID.step(estimateZ);
			m_skyline.setRC(roll, pitch, 1500, throttle);
			break;
		case FlightMode::PositionControl:
			
			estimateX = m_obs->initialPositionX + m_obs->ioPositionX;
			estimateY = m_obs->initialPositionY + m_obs->ioPositionY;
			estimateZ = m_obs->initialPositionZ + m_obs->ioPositionZ;

			estimateX -= m_obs->positionX;
			estimateY -= m_obs->positionY;
			estimateZ -= m_obs->positionZ;

			// use pos pid to get velocity targets.
			vx = m_posXPID.step(estimateX);
			vy = m_posYPID.step(estimateY);

			// use velocity targets to reach velocities.
			roll = (uint16_t)m_rollPID.step(m_obs->velocityX - vx);
			pitch = (uint16_t)m_pitchPID.step(m_obs->velocityY - vy);
			throttle = (uint16_t)m_throttlePID.step(estimateZ);
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
