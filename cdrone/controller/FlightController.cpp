#include <iostream>
#include <stdexcept>

#include "controller/FlightController.h"
#include "misc/logging.h"


FlightController::FlightController(Config &config,
		std::shared_ptr<Observations> obs) : m_obs(obs), 
	m_lastModeChange(std::chrono::high_resolution_clock::now()),
	m_skyline(config, obs), m_rollPID(config.rollPIDP(), config.rollPIDI(),
			config.rollPIDD(), 1500, 1400, 1600), m_pitchPID(config.pitchPIDP(),
		   config.pitchPIDI(), config.pitchPIDD(), 1500, 1400, 1600),
	m_throttlePID(config.throttlePIDP(), config.throttlePIDI(),
			config.throttlePIDD(), 1395, 1100, 1900) {
	m_mode = Disarmed;
}

FlightMode FlightController::getMode() {
	return m_mode.load(std::memory_order_acquire);
}

void FlightController::arm() {
	if (getMode() == Disarmed)
		setMode(Arming);
}

void FlightController::disarm() {
	auto mode = getMode();
	if (mode != Disarmed && mode != Disarming)
		setMode(Disarming);
}

void FlightController::takeoff() {
	auto mode = getMode();
	if (mode == Armed)
		setMode(TakeOff);
}

void FlightController::calibrate() {
	// TODO: set mode to calibrate
	if (m_mode != Disarmed) {
		console->warn("could not calibrate, in mode {}", m_mode);
	}
	// m_mode = Calibrating;
	m_skyline.sendCalibrate();
}

void FlightController::update() {
	uint16_t roll, pitch, throttle;
	double vx, vy;
	double x, y;
	std::chrono::high_resolution_clock::time_point then, now;
	FlightMode mode;

	mode = getMode();
	switch(mode) {
		case FlightMode::Disarmed:
			// don't do anything.. we are disarmed.
			break;
		case FlightMode::Disarming:
			// send a disarm signal to the multiwii board.
			m_skyline.sendDisarm();
			// check if enough time has passed
			then = m_lastModeChange;
			now = std::chrono::high_resolution_clock::now();
			if (now - then > std::chrono::seconds(1))
				setMode(FlightMode::Disarmed);
			break;
		case FlightMode::Arming:
			m_skyline.sendArm();
			then = m_lastModeChange;
			now = std::chrono::high_resolution_clock::now();
			if (now - then > std::chrono::seconds(1)) {
				setMode(FlightMode::Armed);
				m_rollPID.reset();
				m_pitchPID.reset();
				m_throttlePID.reset();
			}
			break;
		case FlightMode::TakeOff:
			/*
			m_skyline.sendRC(m_obs->ioVelocityRoll, m_obs->ioVelocityPitch,
					m_obs->ioVelocityYaw, m_obs->ioVelocityThrottle);
					*/
			vx = ((double)m_obs->ioVelocityRoll - 1500.0) / 1000.0;
			vy = ((double)m_obs->ioVelocityPitch - 1500.0) / 1000.0;
			roll = (uint16_t)m_rollPID.step(m_obs->cameraXMotion - vx);
			pitch = (uint16_t)m_pitchPID.step(m_obs->cameraYMotion - vy);
			throttle = (uint16_t)m_throttlePID.step(m_obs->infraredHeight - 0.2);
			m_skyline.sendRC(roll, pitch,
					m_obs->ioVelocityYaw, throttle);
			x = m_obs->cameraXMotion; y = m_obs->cameraYMotion;
			std::cout << roll << " " << pitch << " " << throttle << std::endl;
			std::cout << x << " " << y << std::endl;
			break;
		case FlightMode::Armed:
			m_skyline.sendIdle();
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
