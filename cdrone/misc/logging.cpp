#include "misc/logging.h"

std::shared_ptr<spdlog::logger> console;

void initialize_logging() {
	console = spdlog::stdout_color_mt("console");
}
