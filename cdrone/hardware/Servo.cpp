#include "Servo.h"

#include <atomic>
#include <mutex>

#include <wiringPi.h>

std::atomic<int> Servo::initialize_number;
std::mutex Servo::initialize_mutex;
// volatile uint32_t* Servo::peripheral;

Servo::Servo(int pin) : Servo(pin, (MIN_VALUE + MAX_VALUE) / 2) {}
Servo::Servo(int pin, int value) :  m_pin(pin), m_value(value) {
	std::lock_guard<std::mutex> lock(initialize_mutex);
	if (!initialize_number++) {
		if (wiringPiSetupGpio() == -1)
			throw std::runtime_error("could not start wiringPiSetup()");
	}
	pinMode(m_pin, PWM_OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetClock(192);
	pwmSetRange(2000);
	setValue(m_value);
}

void Servo::setValue(int value) {
	if (value < MIN_VALUE)
		value = MIN_VALUE;
	if (value > MAX_VALUE)
		value = MAX_VALUE;
	m_value = value;
	
	pwmWrite(m_pin, m_value);
}

Servo::~Servo() {
	std::lock_guard<std::mutex> lock(initialize_mutex);
	if (!--initialize_number){}
}

