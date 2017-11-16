#include "FlightController.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

FlightController::FlightController(Config &config) {
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
	m_skyline->calibrate();
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
	struct timespec ts;
	::clock_gettime(CLOCK_REALTIME, &ts);
	m_timeSinceModeChange = ts;
	SPDLOG_DEBUG(spdlog::get("console"), "MODE CHANGED FROM {} to {}", m_mode, mode);
	m_mode = mode;
}
