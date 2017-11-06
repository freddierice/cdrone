#ifndef __FLIGHT_CONTROLLER_H__
#define __FLIGHT_CONTROLLER_H__
#include <atomic>
#include <time.h>

// FightMode holds the flight modes.
enum FlightMode {Disarmed, Disarming, Arming, Armed, TakeOff, TouchDown, Hover};

class FlightController {
public:
	FlightController();
	FlightController(FlightMode mode);
	
	void arm();
	void disarm();
	void update();
	FlightMode getMode();

private:
	std::atomic<FlightMode> m_mode;
	std::atomic<struct timespec> m_timeSinceModeChange;
	
	void setMode(FlightMode m);
};

#endif /* __FLIGHT_CONTROLLER_H__ */
