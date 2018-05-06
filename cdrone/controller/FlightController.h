#ifndef __FLIGHT_CONTROLLER_H__
#define __FLIGHT_CONTROLLER_H__
#include <atomic>
#include <chrono>
#include <memory>

#include <time.h>

#include "controller/PID.h"
#include "hardware/Skyline.h"
#include "misc/Config.h"
#include "misc/Observations.h"

// FightMode holds the flight modes.
enum FlightMode {Disarmed, Disarming, Arming, Armed, TakeOff, TouchDown, Hover,
	Calibrating, RawControl, VelocityControl, PositionControl};

// FlightController controls an underlying flight controller. The object is
// dependant on a two thread model, a single controller and updater. One thread
// can continously run update and update only. The other thread can run any of
// the other functions. This property allows for the object to be completely
// lock free. XXX: Currently this is not a necessary requirement, but it could
// be necesary some time in the future. 
class FlightController {
public:
	FlightController(Config &config, std::shared_ptr<Observations> obs);
	
	// FUNCTIONS TO BE RUN BY THE CONTROLLER THREAD //
	// arm will set the mode to arm. This function is only valid when the 
	// flight controller is in the disarm state.
	void arm();

	// calibrate will try to run a calibration on the drone. This function is
	// only valid when the drone is in the disarm state.
	void calibrate();

	// disarm will disarm the drone. This is available when the drone is in any
	// state. XXX: be careful, currently this may leave the drone in a weird
	// state if it is re-armed after a rough disarm.
	void disarm();

	// takeoff will let the drone takeoff and move into velocity mode.
	void takeoff();

	// rawMode will put the drone into the raw mode.
	void rawControl();

	// velocityMode will put the drone into the velocity mode.
	void velocityControl();

	// positionMode will put the drone into position mode.
	void positionControl();

	// FUNCTION TO BE RUN BY THE UPDATER THREAD //
	// update will send the signals to the underlying flight controller.
	void updateController();
	void updateRC();
	
	// FUNCTIONS THAT CAN BE RUN BY ANY THREAD //
	FlightMode getMode();

private:
	// global observations made from other hardware.
	std::shared_ptr<Observations> m_obs;
	
	// The current mode of the flight controller. This keeps track of the
	// flight controller state machine.
	std::atomic<FlightMode> m_mode;

	// The last time when the mode was changed. This lets the update thread
	// know when it is time to send the right signals and transition into new
	// states.
	std::atomic<std::chrono::high_resolution_clock::time_point> m_lastModeChange;
	std::atomic<std::chrono::high_resolution_clock::time_point> m_lastSendRC;
	Skyline m_skyline;
	
	FlightController();
	void setMode(FlightMode m);

	PID m_rollPID;
	PID m_pitchPID;
	PID m_throttlePID;
	PID m_posXPID;
	PID m_posYPID;
	PID m_yawPID;
};

#endif /* __FLIGHT_CONTROLLER_H__ */
