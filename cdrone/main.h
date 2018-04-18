#ifndef __MAIN_H__
#define __MAIN_H__
#include <atomic>

// a global atomic that is false for the duration of the program.
namespace global {
	extern std::atomic<bool> shutdown;
}

#endif /* __MAIN_H__ */
