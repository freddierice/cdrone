#include "utility.h"
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <spdlog/spdlog.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


void pthread_block(int sig) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	pthread_sigmask(SIG_BLOCK, &ss, 0);
}

void pthread_unblock(int sig) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, sig);
	pthread_sigmask(SIG_UNBLOCK, &ss, 0);
}

double get_time() {
	struct timespec ts; 
	::clock_gettime(CLOCK_REALTIME, &ts);
	return (double)ts.tv_sec + ((double)ts.tv_nsec)/1000000000;
}

int create_socket(uint16_t port) {
	int fd;
	struct sockaddr_in addr;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		spdlog::get("console")->error("could not create socket");
		return -1;
	}

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		spdlog::get("console")->error("could not bind to the specified address");
		return -1;
	}

	if (listen(fd, 1) < 0) {
		spdlog::get("console")->error("could not set listen queue");
		return -1;
	}

	return fd;
}

bool loaded;
SSL_CTX *ssl_create_context(const char *ca, const char *cert, const char *privkey) {
	if (!loaded) {
		loaded = true;
		SSL_load_error_strings();
		OpenSSL_add_ssl_algorithms();
	}
	
	const SSL_METHOD *method;
	SSL_CTX *ctx;

	// TODO: SSLv3 may be better
	method = SSLv23_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		spdlog::get("console")->error("could not create context");
		ERR_print_errors_fp(stderr);
		return (SSL_CTX *)0;
	}

	if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) != 1) {
		ERR_print_errors_fp(stderr);
		return (SSL_CTX *)0;
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, privkey, SSL_FILETYPE_PEM) != 1) {
		ERR_print_errors_fp(stderr);
		return (SSL_CTX *)0;
	}
	if (!SSL_CTX_check_private_key(ctx)) {
		spdlog::get("console")->error("private key does not match certificate");
	}

	// TODO: we need to add
	// SSL_CTX_use_certificate_chain_file(SSL_CTX *ctx, const char *file);

	return ctx;
}
