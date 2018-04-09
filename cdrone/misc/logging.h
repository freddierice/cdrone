#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <spdlog/spdlog.h>

// Global shared pointer to a logger. Do not use the spdlog::get("") functions
// elsewhere because as of right now it will cause unexpected memory
// corruption on the stack.
extern std::shared_ptr<spdlog::logger> console;

// initialize the logging for cdrone.
void initialize_logging();



#endif /* __LOGGING_H__ */
