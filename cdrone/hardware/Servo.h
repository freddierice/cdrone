#ifndef __SERVO_H__
#define __SERVO_H__

#include <atomic>
#include <mutex>

class Servo {
public:
	Servo(int pin, int value);
	Servo(int pin);
	~Servo();

	void setValue(int value);

	constexpr static int MIN_VALUE = 80;
	constexpr static int MAX_VALUE = 240;
private:
	Servo() = delete;
	void initialize();

	int m_pin;
	int m_value;

	static std::atomic<int> initialize_number;
	static std::mutex initialize_mutex;
};

#endif /* SERVO */
