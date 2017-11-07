#include "FlightController.h"

FlightController::FlightController() : FlightController(Disarmed) {}
FlightController::FlightController(FlightMode mode) : m_mode(mode) {}

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

void FlightController::update() {
	auto mode = getMode();
}

void FlightController::setMode(FlightMode mode) {
	struct timespec ts;
	::clock_gettime(CLOCK_REALTIME, &ts);
	m_timeSinceModeChange = ts;
	m_mode = mode;
}
