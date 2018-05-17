#include "position/VRPN.h"


#include "logging/vrpn.h"
#include "logging/vector3.h"

#include <cmath>
#include <iostream>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

VRPN::VRPN(std::string name, int sensor, std::shared_ptr<Observations> obs) : m_stop(false),
	m_logger("vrpn", &logging::vrpn_variable), m_quatOld_logger("quat_old", &logging::vector3_variable),
	m_quatNew_logger("quat_new", &logging::vector3_variable), m_remote(name.c_str()), m_obs(obs), m_rot(3,3),
	m_thread(&VRPN::do_main, this) {
	m_rot(0,0) = -1;
	m_rot(0,1) = 0;
	m_rot(0,2) = 0;
	m_rot(1,0) = 0;
	m_rot(1,1) = 0;
	m_rot(1,2) = 1;
	m_rot(2,0) = 0;
	m_rot(2,1) = 1;
	m_rot(2,2) = 0;
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
Eigen::Matrix3d rot(3,3);
constexpr double posAlpha = 0.3;
constexpr double posBeta = 1 - posAlpha;
void VRPN_CALLBACK VRPN::callback(void *userdata, const vrpn_TRACKERCB t) {
	VRPN *vrpn_obj = (VRPN *)userdata;
	logging::vrpn_t vrpn;
	double drone_quat_0, drone_quat_1, drone_quat_2, drone_quat_3;
	double drone_vel_x, drone_vel_y, drone_vel_z;
	double drone_pos_x, drone_pos_y, drone_pos_z;
	
	// logging::vector3_t quat_old_log;
	// logging::vector3_t quat_new_log;

	// fill in the vrpn
	vrpn.sensor_time = t.msg_time.tv_sec * 1000000 + t.msg_time.tv_usec;
	vrpn.sensor = t.sensor;
	vrpn.x = t.pos[0]; vrpn.y = t.pos[1]; vrpn.z = t.pos[2];
	vrpn.quat0 = t.quat[0]; vrpn.quat1 = t.quat[1];
	vrpn.quat2 = t.quat[2]; vrpn.quat3 = t.quat[3];
	
	// set drone position
	drone_pos_x = -vrpn.x;
	drone_pos_y = vrpn.z;
	drone_pos_z = vrpn.y;

	// velocity
	drone_vel_x = (drone_pos_x - vrpn_obj->m_obs->positionX)*posAlpha + vrpn_obj->m_obs->velocityX*posBeta;
	drone_vel_y = (drone_pos_y - vrpn_obj->m_obs->positionY)*posAlpha + vrpn_obj->m_obs->velocityY*posBeta;
	drone_vel_z = (drone_pos_z - vrpn_obj->m_obs->positionZ)*posAlpha + vrpn_obj->m_obs->velocityZ*posBeta;

	// set the drone heading
	Eigen::Quaterniond quatDrone(t.quat[0], t.quat[1], t.quat[2], t.quat[3]);
	quatDrone.normalize();
	auto drone_heading = vrpn_obj->m_rot * (quatDrone * Eigen::Vector3d::UnitZ());
	Eigen::Quaterniond drone_quat;
	drone_quat.setFromTwoVectors(drone_heading, Eigen::Vector3d::UnitY());
	drone_quat.normalize();
	drone_quat_0 = drone_quat.w();
	drone_quat_1 = drone_quat.x();
	drone_quat_2 = drone_quat.y();
	drone_quat_3 = drone_quat.z();

	// velocity in the VRPN coordinate system
	vrpn_obj->m_obs->quat0 = drone_quat_0;
	vrpn_obj->m_obs->quat1 = drone_quat_1;
	vrpn_obj->m_obs->quat2 = drone_quat_2;
	vrpn_obj->m_obs->quat3 = drone_quat_3;
	vrpn_obj->m_obs->velocityX = drone_vel_x;
	vrpn_obj->m_obs->velocityY = drone_vel_y;
	vrpn_obj->m_obs->velocityZ = drone_vel_z;
	vrpn_obj->m_obs->positionX = drone_pos_x;
	vrpn_obj->m_obs->positionY = drone_pos_y;
	vrpn_obj->m_obs->positionZ = drone_pos_z;
	vrpn_obj->m_lastTime = vrpn.sensor_time;

	/*
	auto oldQuat = quatDrone * Eigen::Vector3d::UnitY();
	auto newQuat = drone.vec();

	oldQuat.normalize();
	newQuat.normalize();
	
	quat_old_log.x = oldVec(0);
	quat_old_log.y = oldVec(1);
	quat_old_log.z = oldVec(2);

	quat_new_log.x = newVec(0);
	quat_new_log.y = newVec(1);
	quat_new_log.z = newVec(2);

	vrpn_obj->m_quatOld_logger.log((void *)&quat_old_log);
	vrpn_obj->m_quatNew_logger.log((void *)&quat_new_log);
	*/

	// log it
	vrpn_obj->m_logger.log(&vrpn);
}
