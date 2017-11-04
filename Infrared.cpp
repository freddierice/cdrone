#include "Infrared.h"

Infrared::Infrared(double alpha, double k) : m_adc(), m_alpha(alpha), m_k(k) {
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
	m_distance = m_k/(double)m_lastDistance;
}
Infrared::Infrared() : Infrared(0,1) {}

Infrared::~Infrared() {
	m_adc.setConfig(m_defaultConfig);
}

double Infrared::distance() {
	return m_distance;
}

uint16_t Infrared::raw() {
	return m_lastDistance;
}

void Infrared::update() {
	// get latest reading
	m_lastDistance = m_adc.getConversion();

	// if it is a negative value, make it MIN_READING
	m_lastDistance = m_lastDistance & 0x8000 ? MIN_READING : m_lastDistance;

	// if it is less than MIN_READING, make it MIN_READING
	m_lastDistance = m_lastDistance < MIN_READING ? MIN_READING : m_lastDistance;

	// do smoothed distance
	m_distance = (1-m_alpha)*m_distance + m_alpha*(m_k/((double)m_lastDistance*(double)m_lastDistance));
}

const uint16_t Infrared::MIN_READING = 240;
