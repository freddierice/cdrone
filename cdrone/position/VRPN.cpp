#include "position/VRPN.h"

#include "logging/vrpn.h"

VRPN::VRPN(std::string name, int sensor) : m_stop(false), m_thread(&VRPN::do_main, this),
	m_logger("vrpn", &logging::vrpn_variable), m_remote(name.c_str()) {
	m_remote.register_change_handler(this, VRPN::callback, sensor);
}

VRPN::~VRPN() {
	m_stop = true;
	m_thread.join();
}

void VRPN::do_main() {
	while (m_stop) {
		m_remote.mainloop();
	}
}

void VRPN_CALLBACK VRPN::callback(void *userdata, const vrpn_TRACKERCB t) {
	VRPN *vrpn_obj = (VRPN *)userdata;
	logging::vrpn_t vrpn;

	// fill in the vrpn
	vrpn.sensor_time = t.msg_time.tv_sec * 1000000 + t.msg_time.tv_usec;
	vrpn.sensor = t.sensor;
	vrpn.x = t.pos[0]; vrpn.y = t.pos[1]; vrpn.z = t.pos[2];
	vrpn.quat0 = t.quat[0]; vrpn.quat1 = t.quat[1];
	vrpn.quat2 = t.quat[2]; vrpn.quat3 = t.quat[3];
	
	// log it
	vrpn_obj->m_logger.log(&vrpn);
}
