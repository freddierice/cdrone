#ifndef __INFRARED_H__
#define __INFRARED_H__

#include "ADS1115.h"
#include <inttypes.h>

class Infrared {
public:
	Infrared();
	Infrared(double alpha, double k);
	~Infrared();

	double distance();
	uint16_t raw();
	void update();
private:
	ADS1115 m_adc;
	uint16_t m_defaultConfig;
	uint16_t m_lastDistance;
	double m_distance;
	double m_alpha;
	double m_k;
	static const uint16_t MIN_READING;
};

#endif /* __INFRARED_H__ */
