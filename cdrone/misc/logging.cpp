#include "misc/logging.h"

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

	void write_map(std::string name, int id, int size) {
		proto::Map map;
		map.Clear();
		map.set_name(name);
		map.set_id((uint32_t)id);
		map.set_size((uint32_t)size);

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
		logging::mappings.close();

		struct archive *a;
		int current = 0;

		a = archive_write_new();
		archive_write_add_filter_gzip(a);
		archive_write_set_format_pax_restricted(a);
		archive_write_open_filename(a, (logging::logging_name + ".logz").c_str());
		logging::archive_add(a, "map");
		while(logging::archive_add(a, std::to_string(current++))) {}
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
}
