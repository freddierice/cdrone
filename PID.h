#ifndef __PID_H__
#define __PID_H__
#include <time.h>

class PID {
public:
	PID();
	PID(double p, double i, double d);
	PID(double p, double i, double d, double min, double max);

	double step(double err);
	void reset();
private:
	double m_p, m_i, m_d;
	double m_min, m_max;
	double m_integral;
	double m_lastTime, m_lastErr;

	static double getTime();
};

#endif /*__PID_H__*/
