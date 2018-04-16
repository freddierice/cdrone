#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <spdlog/spdlog.h>

#include <fstream>
#include <string>
#include <vector>

// Global shared pointer to a logger. Do not use the spdlog::get("") functions
// elsewhere because as of right now it will cause unexpected memory
// corruption on the stack.
extern std::shared_ptr<spdlog::logger> console;

// initialize the logging for cdrone.
void initialize_logging();

class Variable;
class archive;
namespace logging {
	extern std::mutex constructor_lock;
	extern std::string logging_name;
	extern std::ofstream mappings;
	extern int created, destroyed;

	extern void initialize_variable_logging();
	extern void clean_variable_logging();
	extern bool archive_add(archive *a, std::string name);
	extern void write_map(Variable name, int id);


	class Variable {
	public:
		virtual std::string name() {
			throw std::runtime_error("name() not implemented.");
		};
		virtual unsigned int size() {
			throw std::runtime_error("size() not implemented.");
		};
		virtual std::vector<std::string> names() {
			throw std::runtime_error("names() not implemented.");
		};
		virtual std::vector<std::string> types() {
			throw std::runtime_error("types() not implemented.");
		}
	};

	class VariableLogger {
	public:
		VariableLogger(std::string name, Variable variable);
		~VariableLogger();
		
		void log(void *variable);

	private:
		void flush();

		const std::string m_name;
		const unsigned int m_size;
		int m_id;
		int m_fd;
		char m_buffer[8192];
		char *m_ptr;
	};
}

#endif /* __LOGGING_H__ */
