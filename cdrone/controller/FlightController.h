#ifndef __FLIGHT_CONTROLLER_H__
#define __FLIGHT_CONTROLLER_H__
#include <atomic>
#include <chrono>
#include <memory>

#include <time.h>

#include "hardware/Skyline.h"
#include "misc/Config.h"

// FightMode holds the flight modes.
enum FlightMode {Disarmed, Disarming, Arming, Armed, TakeOff, TouchDown, Hover,
	Calibrating};

// FlightController controls an underlying flight controller. The object is
// dependant on a two thread model, a single controller and updater. One thread
// can continously run update and update only. The other thread can run any of
// the other functions. This property allows for the object to be completely
// lock free. XXX: Currently this is not a necessary requirement, but it could
// be necesary some time in the future. 
class FlightController {
public:
	FlightController(Config &config);
	
	// FUNCTIONS TO BE RUN BY THE CONTROLLER THREAD //
	// arm will set the mode to arm. This function is only valid when the 
	// flight controller is in the disarm state.
	void arm();

	// calibrate will try to run a calibration on the drone. This function is
	// only valid when the drone is in the disarm state.
	bool calibrate();

	// disarm will disarm the drone. This is available when the drone is in any
	// state. XXX: be careful, currently this may leave the drone in a weird
	// state if it is re-armed after a rough disarm.
	void disarm();


	// FUNCTION TO BE RUN BY THE UPDATER THREAD //
	// update will send the signals to the underlying flight controller.
	void update();
	
	// FUNCTIONS THAT CAN BE RUN BY ANY THREAD //
	FlightMode getMode();

private:
	// The current mode of the flight controller. This keeps track of the
	// flight controller state machine.
	std::atomic<FlightMode> m_mode;

	// The last time when the mode was changed. This lets the update thread
	// know when it is time to send the right signals and transition into new
	// states.
	std::atomic<std::chrono::high_resolution_clock::time_point> m_lastModeChange;
	std::unique_ptr<Skyline> m_skyline;
	
	FlightController();
	void setMode(FlightMode m);
};

#endif /* __FLIGHT_CONTROLLER_H__ */
