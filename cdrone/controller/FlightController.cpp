#include "FlightController.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

FlightController::FlightController(Config &config) : 
	m_lastModeChange(std::chrono::high_resolution_clock::now()) {
	Serial skylineSerial(config.skylinePort());
	m_skyline = std::make_unique<Skyline>(skylineSerial);
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

bool FlightController::calibrate() {
	// TODO: set mode to calibrate
	if (m_mode != Disarmed) {
		spdlog::get("console")->warn("could not calibrate, in mode {}", m_mode);
		return false;
	}
	m_mode = Calibrating;
	m_skyline->sendCalibrate();
	return true;
}

void FlightController::update() {
	auto mode = getMode();
	switch(mode) {
		case FlightMode::Disarmed:
			// don't do anything.. we are disarmed.
			break;
		case FlightMode::Disarming:
			// send a disarm signal to the multiwii board.
			m_skyline->sendDisarm();
			// check if enough time has passed
			//
			break;
		case FlightMode::Arming:
			break;
		case FlightMode::Armed:
			break;
		case FlightMode::TakeOff:
			break;
		case FlightMode::TouchDown:
			break;
		case FlightMode::Hover:
			break;
		default:
			SPDLOG_DEBUG(spdlog::get("console"), "unhandled mode: {}", mode);
			throw std::runtime_error("got into a bad mode");
	}
}

void FlightController::setMode(FlightMode mode) {
	m_lastModeChange = std::chrono::high_resolution_clock::now();
	SPDLOG_DEBUG(spdlog::get("console"), "MODE CHANGED FROM {} to {}", m_mode, mode);
	m_mode = mode;
}
