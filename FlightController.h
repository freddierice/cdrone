#ifndef __FLIGHT_CONTROLLER_H__
#define __FLIGHT_CONTROLLER_H__

// FightMode holds the flight modes.
enum FlightMode {Disarmed, Armed, TakeOff, TouchDown, Hover};

class FlightController {
public:
	FlightController();
	
	void arm();
	void disarm();

private:
	FlightMode m_mode;
};

#endif /* __FLIGHT_CONTROLLER_H__ */
