#ifndef __INFRARED_H__
#define __INFRARED_H__

#include "ADS1115.h"
#include <inttypes.h>

class Infrared {
public:
	Infrared();
	~Infrared();
	uint16_t distance();
private:
	ADS1115 m_adc;
	uint16_t m_defaultConfig;
};

#endif /* __INFRARED_H__ */
