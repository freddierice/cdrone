#include "Infrared.h"

Infrared::Infrared() : m_adc() {
	// save the ADC old config
	m_defaultConfig = m_adc.getConfig();

	// set to continuous mode
	m_adc.setConfig((uint16_t)(	
				ADS1115::Config::MUX_SINGLE_0 | 
				ADS1115::Config::DR_128SPS    | 
				ADS1115::Config::CQUE_NONE    |
				ADS1115::Gain::ONE));
}

Infrared::~Infrared() {
	m_adc.setConfig(m_defaultConfig);
}

uint16_t Infrared::distance() {
	return m_adc.getConversion();
}
