#include "hardware/Infrared.h"

Infrared::Infrared(Config &config, std::shared_ptr<Observations> obs) : 
	Infrared(config.infraredAlpha(), config.infraredB(), config.infraredK(), obs) {}

Infrared::Infrared(double alpha, double b, double k) : 
	Infrared(alpha, b, k, std::make_shared<Observations>()) {}

Infrared::Infrared(double alpha, double b, double k,
		std::shared_ptr<Observations> obs) : m_adc(), m_alpha(alpha),
	m_b(b), m_k(k), m_obs(obs) {

	// save old default
	m_defaultConfig = m_adc.getConfig();

	// set the Gain to ~4V
	// read from channel 0 TODO: configure other channels.
	// read at better than 70 Hz (that is the best case scenario
	// as stated by the datasheet).
	m_adc.setConfig(
			ADS1115::Gain::ONE | 
			ADS1115::Config::MUX_SINGLE_0 |
			ADS1115::Config::DR_128SPS |
			ADS1115::Config::CQUE_NONE);

	// get in the ballpark before launching the smoothed infrared.
	update(); update(); update(); update(); update(); update();
}
Infrared::Infrared() : Infrared(0,1,0) {}

Infrared::~Infrared() {
	m_adc.setConfig(m_defaultConfig);
}

void Infrared::update() {
	double newDistance, oldDistance;
	uint16_t newVoltage;

	// get latest reading
	newVoltage = m_adc.getConversion();

	// if it is a negative value, make it MIN_READING
	// if it is less than MIN_READING, make it MIN_READING
	newVoltage = newVoltage & 0x8000 ? MIN_READING : newVoltage;
	newVoltage = newVoltage < MIN_READING ? MIN_READING : newVoltage;

	// convert the voltage to raw distance
	newDistance = m_k/(double)newVoltage + m_b;

	// do smoothed distance
	oldDistance = m_obs->infraredHeight;
	newDistance = (1-m_alpha)*oldDistance + m_alpha*newDistance;

	// update the atomics
	m_obs->infraredHeight = newDistance;
	m_obs->infraredVoltage = newVoltage;
}

const uint16_t Infrared::MIN_READING = 2000;
