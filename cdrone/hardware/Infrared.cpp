#include "Infrared.h"

Infrared::Infrared(Config &config) : 
	Infrared(config.infraredAlpha(), config.infraredB(), config.infraredK()) {}

Infrared::Infrared(double alpha, double b, double k) : m_adc(), m_alpha(alpha),
	m_b(b), m_k(k) {
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
	update();
	m_distance = m_k/(double)m_voltage + m_b;
}
Infrared::Infrared() : Infrared(0,1,0) {}

Infrared::~Infrared() {
	m_adc.setConfig(m_defaultConfig);
}

double Infrared::distance() {
	return m_distance.load(std::memory_order_acquire);
}

uint16_t Infrared::voltage() {
	return m_voltage.load(std::memory_order_acquire);
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
	oldDistance = m_distance.load(std::memory_order_acquire);
	newDistance = (1-m_alpha)*oldDistance + m_alpha*newDistance;

	// update the atomics
	m_distance.store(newDistance, std::memory_order_relaxed);
	m_voltage.store(newVoltage, std::memory_order_release);
}

const uint16_t Infrared::MIN_READING = 2000;
