#ifndef __FLIGHT_CONTROLLER_H__
#define __FLIGHT_CONTROLLER_H__
#include <atomic>
#include <memory>

#include <time.h>

#include "hardware/Skyline.h"
#include "misc/Config.h"

// FightMode holds the flight modes.
enum FlightMode {Disarmed, Disarming, Arming, Armed, TakeOff, TouchDown, Hover};

class FlightController {
public:
	FlightController(Config &config);
	
	void arm();
	bool calibrate();
	void disarm();
	void update();
	FlightMode getMode();

private:
	std::atomic<FlightMode> m_mode;
	std::atomic<struct timespec> m_timeSinceModeChange;
	std::unique_ptr<Skyline> m_skyline;
	
	FlightController();
	void setMode(FlightMode m);
};

#endif /* __FLIGHT_CONTROLLER_H__ */
