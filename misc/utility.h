#ifndef __UTILITY_H__
#define __UTILITY_H__
#include <inttypes.h>
#include <openssl/ssl.h>

void pthread_block(int sig);
void pthread_unblock(int sig);
double get_time();

SSL_CTX *ssl_create_context(const char *);

int create_socket(uint16_t port);

#endif /* __UTILITY_H__ */
