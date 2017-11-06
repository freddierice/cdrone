#include "PID.h"
#include <limits>
#include <time.h>
#include "utility.h"

PID::PID() : PID(0,0,0) {}
PID::PID(double p, double i, double d) : PID(p,i,d,
		-1.0*std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::infinity()){}
PID::PID(double p, double i, double d, double min, double max) : m_p(p), 
	m_i(i), m_d(d), m_min(min), m_max(max) {
	reset();
}

double PID::step(double err) {
	double all, now, deltaTime;

	now = ::get_time();
	deltaTime = now - m_lastTime;

	// add up all the terms
	all = m_p*err + m_i*m_integral + m_d*(err - m_lastErr)/deltaTime;
	m_integral += err*deltaTime;
	m_lastTime = now;
	m_lastErr = err;

	// make sure we are within the bounds.
	all = all > m_max ? m_max : all;
	all = all < m_min ? m_min : all;

	return all;
}

void PID::reset() {
	m_integral = 0.0;
}
