#ifndef __ADS1115_H__
#define __ADS1115_H__

#include "I2C.h"
#include <inttypes.h>

class ADS1115 {
public:
	ADS1115();
	ADS1115(int channel);
	ADS1115(int channel, int address);

	void setConfig(uint16_t config);
	uint16_t getConfig();
	uint16_t getConversion();

	static const int DEFAULT_ADDRESS;
	enum Gain {
		TWOTHIRDS = 0x0000, // 6.144V range
		ONE       = 0x0200, // 4.096V range
		TWO       = 0x0400, // 2.048V range
		FOUR      = 0x0600, // 1.024V range
		EIGHT     = 0x0800, // 0.512V range
		SIXTEEN   = 0x0A00, // 0.256V range
	};
	enum Register {
    	CONVERT   = 0x00,
		CONFIG    = 0x01,
		LOWTHRESH = 0x02,
		HITHRESH  = 0x03,
	};
	enum Config {
		OS_BUSY    = 0x0000, // Bit = 0 when conversion is in progress
		OS_NOTBUSY = 0x8000, // Bit = 1 when conversion is not in progress

		// Differential P = AIN0, N = AIN1 (default)
		MUX_DIFF_0_3 = 0x1000,  // Differential P = AIN0, N = AIN3
		MUX_DIFF_1_3 = 0x2000,  // Differential P = AIN1, N = AIN3
		MUX_DIFF_2_3 = 0x3000,  // Differential P = AIN2, N = AIN3
		MUX_SINGLE_0 = 0x4000,  // Single-ended AIN0
		MUX_SINGLE_1 = 0x5000,  // Single-ended AIN1
		MUX_SINGLE_2 = 0x6000,  // Single-ended AIN2
		MUX_SINGLE_3 = 0x7000,  // Single-ended AIN3
		
		// continuous by default
		MODE_SINGLE = 0x0100,  // Power-down single-shot mode (default)

		// 8 samples per second by default
		DR_16SPS   = 0x0020,  // 16 samples per second
		DR_32SPS   = 0x0040,  // 32 samples per second
		DR_64SPS   = 0x0060,  // 64 samples per second
		DR_128SPS  = 0x0080,  // 128 samples per second (default)
		DR_250SPS  = 0x00A0,  // 250 samples per second
		DR_475SPS  = 0x00C0,  // 475 samples per second
		DR_860SPS  = 0x00E0,  // 860 samples per second

		// traditional with hysteresis by default
		CMODE_WINDOW= 0x0010,  // Window comparator 

		// default low
		CPOL_ACTVHI = 0x0008,  // ALERT/RDY pin is high when active 

		// default non-latching
		CLAT_LATCH  = 0x0004,  // Latching comparator 

		// default ALERT after 1 conversion
    	CQUE_2CONV = 0x0001,  // Assert ALERT/RDY after two conversions
    	CQUE_4CONV = 0x0002,  // Assert ALERT/RDY after four conversions
    	CQUE_NONE  = 0x0003,  // Disable the comparator and put ALERT/RDY in high state (default)
	};
private:
	I2C m_i2c;
	int m_channel;
};

#endif /* __ADS1115_H__ */
