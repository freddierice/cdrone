#include <chrono>
#include <iostream>
#include <limits>

#include "controller/PID.h"
#include "misc/utility.h"
// #include "misc/logging.h"


PID::PID() : PID(0,0,0) {}
PID::PID(double p, double i, double d) : PID(p,i,d,0) {}
PID::PID(double p, double i, double d, double k) : PID(p,i,d,k,
		-1.0*std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::infinity(), 0){}
PID::PID(double p, double i, double d, double k, double min, double max) : 
	PID(p, i, d, k, min, max, 0) {}
PID::PID(double p, double i, double d, double k, double min, double max, double delay) : 
	m_p(p), m_i(i), m_d(d), m_k(k), m_min(min), m_max(max), m_delay(delay) { 
	reset();
}

double PID::step(double err) {
	double all, deltaTime, errDelta;

	// std::cout << m_p << " " << m_i << " " << m_d << " " << m_k << " " << m_min << " " << m_max <<  std::endl;

	auto now = std::chrono::high_resolution_clock::now();
	deltaTime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastTime).count());

	if (deltaTime < m_delay)
		return m_lastOut;

	// only use a d term if it is moving toward target
	errDelta = err - m_lastErr;

	// add up all the terms
	all = m_p*err + m_i*m_integral + m_d*errDelta + m_k;
	m_integral += err;
	m_lastTime = now;
	m_lastErr = err;

	// make sure we are within the bounds.
	all = all > m_max ? m_max : all;
	all = all < m_min ? m_min : all;

	m_lastOut = all;

	return all;
}

void PID::reset() {
	m_lastErr = 0.0;
	m_lastTime = std::chrono::high_resolution_clock::now();
	m_integral = 0.0;
	m_lastOut = m_k;
}
