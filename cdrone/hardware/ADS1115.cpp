#include "hardware/ADS1115.h"

ADS1115::ADS1115(int channel, int address) : m_i2c(address), m_channel(channel) {}
ADS1115::ADS1115(int channel) : ADS1115(0, ADS1115::DEFAULT_ADDRESS) {}
ADS1115::ADS1115() : ADS1115(0) {}

void ADS1115::setConfig(uint16_t config) {
	m_i2c.writeRegister((uint8_t)Register::CONFIG, config);
}

uint16_t ADS1115::getConfig() {
	return m_i2c.readRegister((uint8_t)Register::CONFIG);
}

uint16_t ADS1115::getConversion() {
	return m_i2c.readRegister((uint8_t)Register::CONVERT);
}

const int ADS1115::DEFAULT_ADDRESS = 0x48;
