#include "logging/logging.h"

#include <proto/log.pb.h>

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <mutex>

#include <archive.h>
#include <archive_entry.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <dirent.h>

/// CONSOLE LOGGING
std::shared_ptr<spdlog::logger> console;

void initialize_logging() {
	console = spdlog::stdout_color_mt("console");
}

/// BINARY LOGGING
namespace logging {
	std::mutex constructor_lock;
	std::string logging_name;
	std::ofstream mappings;
	int created = 0;
	int destroyed = 0;

	void initialize_variable_logging() {
		auto now = std::chrono::high_resolution_clock::now();
		logging_name = "run_" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
		if (mkdir(logging_name.c_str(), 0755)) {
			throw std::exception();
		}
	
		mappings = std::ofstream(logging_name + "/" + "map", std::ofstream::out);
	}

	void write_map(std::string name, Variable var, int id) {
		proto::Map map;
		map.Clear();
		map.set_name(name);
		map.set_id((uint32_t)id);
		map.set_size((uint32_t)var.size());

		for (auto var_name = var.names().begin(); var_name != var.names().end(); var_name++)
			map.add_var_names(*var_name);
		for (auto type_name = var.types().begin(); type_name != var.types().end(); type_name++)
			map.add_type_names(*type_name);

		uint32_t len_send = htonl(map.ByteSize());
		mappings.write((const char *)&len_send, sizeof(len_send));

		int len = map.ByteSize();
		char buf[len];
		map.SerializeToArray(buf, len);
		mappings.write(buf, len);
	}

	bool archive_add(archive *a, std::string name) {
		std::ifstream file(logging_name + "/" + name);
		if (!file)
			return false;

		file.seekg(0, file.end);
		unsigned int size = file.tellg();
		file.seekg(0, file.beg);

		archive_entry *entry = archive_entry_new();
		archive_entry_set_pathname(entry, name.c_str());
		archive_entry_set_size(entry, size);
		archive_entry_set_filetype(entry, AE_IFREG);
		archive_entry_set_perm(entry, 0644);
		archive_write_header(a, entry);
		while (size > 0) {
			char buf[4096];
			int n = size > sizeof(buf) ? sizeof(buf) : size;
			file.read(buf, n);
			archive_write_data(a, buf, n);
			size -= n;
		}
		file.close();
		archive_entry_free(entry);
		return true;
	}

	void clean_variable_logging() {
		mappings.close();

		struct archive *a;
		int current = 0;

		a = archive_write_new();
		archive_write_add_filter_gzip(a);
		archive_write_set_format_pax_restricted(a);
		archive_write_open_filename(a, (logging_name + ".logz").c_str());
		archive_add(a, "map");
		while(archive_add(a, std::to_string(current++))) {}
		archive_write_close(a);
		archive_write_free(a);

		// delete the old files
		int dir_fd = open(logging_name.c_str(), O_RDWR | O_DIRECTORY);
		DIR* dir = opendir(logging_name.c_str());
		struct dirent *entry;
		while((entry = readdir(dir)) != NULL) {
			unlinkat(dir_fd, entry->d_name, 0);
		}
		closedir(dir);
		::close(dir_fd);

		dir_fd = open(".", O_RDWR | O_DIRECTORY);
		unlinkat(dir_fd, logging_name.c_str(), AT_REMOVEDIR);
		::close(dir_fd);
	}

	VariableLogger::VariableLogger(std::string name, Variable variable) : m_name(name), m_size(variable.size()) {
		std::lock_guard<std::mutex> lock(constructor_lock);
		// the first logger needs to initialize the library
		if (created == destroyed)
			initialize_variable_logging();

		m_id = created++;
		write_map(m_name, variable, m_id);
		if ((m_fd = open((logging_name + "/" + std::to_string(m_id)).c_str(), O_RDWR | O_CREAT, 0644)) == -1) {
			throw std::exception();
		}
		m_ptr = (char *)m_buffer;
	}

	VariableLogger::~VariableLogger() {
		std::lock_guard<std::mutex> lock(constructor_lock);
		flush();
		::close(m_fd);
		destroyed++;
		if (destroyed != created)
			return;

		// flush all and zip up the logs
		clean_variable_logging();
	}

	void VariableLogger::log(void *variable) {
		uint64_t time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		if (m_ptr - m_buffer + m_size + sizeof(time) > sizeof(m_buffer)) {
			flush();
		}
		*(uint64_t *)m_ptr = time;
		m_ptr += sizeof(time);
		memcpy(m_ptr, (void *)variable, m_size);
		m_ptr += m_size;
	}

	void VariableLogger::flush() {
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

}
