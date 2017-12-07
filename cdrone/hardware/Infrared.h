#ifndef __INFRARED_H__
#define __INFRARED_H__

#include <atomic>
#include <inttypes.h>
#include <memory>

#include "hardware/ADS1115.h"
#include "misc/Config.h"
#include "misc/Observations.h"

class Infrared {
public:
	Infrared();
	Infrared(Config &config, std::shared_ptr<Observations> obs);
	Infrared(double alpha, double b, double k);
	Infrared(double alpha, double b, double k, std::shared_ptr<Observations> obs);
	~Infrared();

	void update();

private:
	ADS1115 m_adc;
	uint16_t m_defaultConfig;
	double m_alpha;
	double m_b;
	double m_k;
	
	// observations
	std::shared_ptr<Observations> m_obs;
	
	static const uint16_t MIN_READING;
};

#endif /* __INFRARED_H__ */
