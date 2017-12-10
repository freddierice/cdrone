#ifndef __UTILITY_H__
#define __UTILITY_H__
#include <inttypes.h>
#include <openssl/ssl.h>

// block/unblock sig on the current pthread.
void pthread_block(int sig);
void pthread_unblock(int sig);

// get the time in seconds, with micro granularity.
double get_time();

#endif /* __UTILITY_H__ */
