#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> console;

void initialize_logging();

#endif /* __LOGGING_H__ */
