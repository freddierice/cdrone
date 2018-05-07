#include "position/VRPN.h"

#include "logging/vrpn.h"

#include <cmath>
#include <iostream>

#include <eigen3/Eigen/Geometry>

VRPN::VRPN(std::string name, int sensor, std::shared_ptr<Observations> obs) : m_stop(false),
	m_logger("vrpn", &logging::vrpn_variable), m_remote(name.c_str()), m_obs(obs),
	m_thread(&VRPN::do_main, this) {
	// m_remote.register_change_handler(this, VRPN::callback, sensor);
	m_remote.register_change_handler(this, VRPN::callback);
}

VRPN::~VRPN() {
	m_stop = true;
	m_thread.join();
}

void VRPN::do_main() {
	while (!m_stop) {
		m_remote.mainloop();
	}
}

Eigen::Quaterniond quatX = Eigen::AngleAxisd(0, Eigen::Vector3d::UnitX()) * 
	Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY()) *
	Eigen::AngleAxisd(1, Eigen::Vector3d::UnitZ());
Eigen::Quaterniond quatZ = Eigen::AngleAxisd(0, Eigen::Vector3d::UnitX()) * 
	Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY()) *
	Eigen::AngleAxisd(1, Eigen::Vector3d::UnitZ());
constexpr double posAlpha = 0.2;
constexpr double posBeta = 1 - posAlpha;
void VRPN_CALLBACK VRPN::callback(void *userdata, const vrpn_TRACKERCB t) {
	VRPN *vrpn_obj = (VRPN *)userdata;
	logging::vrpn_t vrpn;
	
	// fill in the vrpn
	vrpn.sensor_time = t.msg_time.tv_sec * 1000000 + t.msg_time.tv_usec;
	vrpn.sensor = t.sensor;
	vrpn.x = t.pos[0]; vrpn.y = t.pos[1]; vrpn.z = t.pos[2];
	vrpn.quat0 = t.quat[0]; vrpn.quat1 = t.quat[1];
	vrpn.quat2 = t.quat[2]; vrpn.quat3 = t.quat[3];

	double velX = (vrpn.x - vrpn_obj->m_obs->positionX)*posAlpha + vrpn_obj->m_obs->velocityX*posBeta;
	double velY = (vrpn.z - vrpn_obj->m_obs->positionY)*posAlpha + vrpn_obj->m_obs->velocityY*posBeta;
	double velZ = (vrpn.y - vrpn_obj->m_obs->positionZ)*posAlpha + vrpn_obj->m_obs->velocityZ*posBeta;

	// velocity in the VRPN coordinate system
	vrpn_obj->m_obs->quat0 = vrpn.quat0;
	vrpn_obj->m_obs->quat1 = vrpn.quat1;
	vrpn_obj->m_obs->quat2 = vrpn.quat2;
	vrpn_obj->m_obs->quat3 = vrpn.quat3;
	vrpn_obj->m_obs->velocityX = velX;
	vrpn_obj->m_obs->velocityY = velY;
	vrpn_obj->m_obs->velocityZ = velZ;
	vrpn_obj->m_obs->positionX = vrpn.x;
	vrpn_obj->m_obs->positionY = vrpn.z;
	vrpn_obj->m_obs->positionZ = vrpn.y;
	vrpn_obj->m_lastTime = vrpn.sensor_time;
	
	// log it
	vrpn_obj->m_logger.log(&vrpn);
}
