#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <spdlog/spdlog.h>

#include <fstream>
#include <string>

// Global shared pointer to a logger. Do not use the spdlog::get("") functions
// elsewhere because as of right now it will cause unexpected memory
// corruption on the stack.
extern std::shared_ptr<spdlog::logger> console;

class archive;
namespace logging {
	extern std::mutex constructor_lock;
	extern std::string logging_name;
	extern std::ofstream mappings;
	extern int created, destroyed;

	extern void initialize_variable_logging();
	extern void clean_variable_logging();
	extern bool archive_add(archive *a, std::string name);
	extern void write_map(std::string name, int id, int size);
}

// initialize the logging for cdrone.
void initialize_logging();

template <class T>
class VariableLogger {
public:
	VariableLogger(std::string name);
	~VariableLogger();

	void log(const T& variable);
	void log(T *variable);

private:
	void flush();

	int m_id;
	std::string m_name;
	int m_fd;
	char m_buffer[8192];
	char *m_ptr;
};

template <class T>
VariableLogger<T>::VariableLogger(std::string name) {
	std::lock_guard<std::mutex> lock(logging::constructor_lock);
	// the first logger needs to initialize the library
	if (logging::created == logging::destroyed)
		logging::initialize_variable_logging();

	m_id = logging::created++;
	m_name = name;
	logging::write_map(name, m_id, sizeof(T));
	if ((m_fd = open((logging::logging_name + "/" + std::to_string(m_id)).c_str(), O_RDWR | O_CREAT, 0644)) == -1) {
		throw std::exception();
	}
	m_ptr = (char *)m_buffer;
}

template<class T>
VariableLogger<T>::~VariableLogger() {
	std::lock_guard<std::mutex> lock(logging::constructor_lock);
	flush();
	::close(m_fd);
	logging::destroyed++;
	if (logging::destroyed != logging::created)
		return;

	// flush all and zip up the logs
	logging::clean_variable_logging();
}

template <class T>
void VariableLogger<T>::log(const T& variable) {
	uint64_t time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	if (m_ptr - m_buffer + sizeof(T) + sizeof(time) > sizeof(m_buffer)) {
		flush();
	}
	*(uint64_t *)m_ptr = time;
	m_ptr += sizeof(time);
	memcpy(m_ptr, (void *)&variable, sizeof(T));
	m_ptr += sizeof(T);
}

template <class T>
void VariableLogger<T>::log(T *variable) {
	uint64_t time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	if (m_ptr - m_buffer + sizeof(T) + sizeof(time) > sizeof(m_buffer)) {
		flush();
	}
	*(uint64_t *)m_ptr = time;
	m_ptr += sizeof(time);
	memcpy(m_ptr, (void *)variable, sizeof(T));
	m_ptr += sizeof(T);
}

template <class T>
void VariableLogger<T>::flush() {
	if (m_ptr - m_buffer == 0)
		return;
	int total = 0;
	while (m_ptr - m_buffer - total) {
		int len = write(m_fd, (void *)(m_buffer+total), m_ptr - m_buffer - total);
		if (len == -1) {
			throw std::exception();
		}
		total += len;
	}
	m_ptr = m_buffer;
}

#endif /* __LOGGING_H__ */
