#ifndef __INFRARED_H__
#define __INFRARED_H__

#include "ADS1115.h"

#include <atomic>
#include <inttypes.h>

class Infrared {
public:
	Infrared();
	Infrared(double alpha, double b, double k);
	~Infrared();

	void update();

	double distance();
	uint16_t voltage();

private:
	ADS1115 m_adc;
	uint16_t m_defaultConfig;
	double m_alpha;
	double m_b;
	double m_k;
	
	// atomic for variables 
	std::atomic<double> m_distance;
	std::atomic<uint16_t> m_voltage;
	
	static const uint16_t MIN_READING;
};

#endif /* __INFRARED_H__ */
