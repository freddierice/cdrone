#ifndef __VRPN_H__
#define __VRPN_H__

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vrpn_Tracker.h>
#include "logging/vrpn.h"
#include "misc/Observations.h"

class VRPN {
public:
	VRPN(std::string name, int sensor, std::shared_ptr<Observations> obs);
	~VRPN();

private:
	void do_main();
	static void VRPN_CALLBACK callback(void *, const vrpn_TRACKERCB t);
	
	std::atomic<bool> m_stop;
	logging::VariableLogger m_logger;
	vrpn_Tracker_Remote m_remote;
	std::shared_ptr<Observations> m_obs;
	uint64_t m_lastTime;
	std::thread m_thread;
};

#endif
