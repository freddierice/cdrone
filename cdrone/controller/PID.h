#ifndef __PID_H__
#define __PID_H__
#include <chrono>
#include <time.h>

class PID {
public:
	PID();
	PID(double p, double i, double d);
	PID(double p, double i, double d, double k);
	PID(double p, double i, double d, double k, double min, double max);
	PID(double p, double i, double d, double k, double min, double max, double delay);

	double step(double err);
	void reset();
private:
	double m_p, m_i, m_d;
	double m_k;
	double m_min, m_max;
	double m_integral;
	std::chrono::high_resolution_clock::time_point m_lastTime;
	double m_lastErr;
	double m_delay;
	double m_lastOut;

	static double getTime();
};

#endif /*__PID_H__*/
