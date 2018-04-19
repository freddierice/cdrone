#ifndef __VRPN_H__
#define __VRPN_H__

#include <atomic>
#include <string>
#include <thread>
#include <vrpn_Tracker.h>
#include "logging/vrpn.h"

class VRPN {
public:
	VRPN(std::string name, int sensor);
	~VRPN();

private:
	void do_main();
	static void VRPN_CALLBACK callback(void *, const vrpn_TRACKERCB t);
	
	std::atomic<bool> m_stop;
	logging::VariableLogger m_logger;
	vrpn_Tracker_Remote m_remote;
	std::thread m_thread;
};

#endif
