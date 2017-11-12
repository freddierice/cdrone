#include "wire/Server.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <spdlog/spdlog.h>

Server::Server(uint16_t port, const std::string& certificate,
		const std::string& key) : m_ssl(0), m_certificate(certificate), 
		m_key(key), m_client_fd(0), m_port(port) {
	init();
}

Server::~Server() {
	cleanup();
}

void Server::reset() {
	cleanup();
	init();
}

void Server::init() {
	m_ctx = ssl_create_context(m_certificate.c_str(), m_key.c_str());
	m_server_fd = create_socket(m_port);
}

void Server::cleanup() {
	if (m_ssl) {
		SSL_free(m_ssl);
		m_ssl = 0;
	}
	if (m_client_fd > 0) {
		::close(m_client_fd);
		m_client_fd = 0;
	}
	if (m_server_fd > 0) {
		::close(m_server_fd);
		m_server_fd = 0;
	}
	if (m_ctx) {
		SSL_CTX_free(m_ctx);
		m_ctx = (SSL_CTX*)0;
	}
}

void Server::accept() {
	if ((m_client_fd = ::accept(m_server_fd, NULL, NULL)) < 0)
		throw ServerException("could not accept client");
	m_ssl = SSL_new(m_ctx);
	SSL_set_fd(m_ssl, m_client_fd);
	if (SSL_accept(m_ssl) <= 0)
		throw ServerException("SSL could not accept client");
}

// TODO: make non-blocking
void Server::readAll(void *buf, int size) {
	int num_read = 0;
	while(num_read < size) {
		int partial_read;
		if ((partial_read = SSL_read(m_ssl, (void *)((char *)buf+num_read),
						size-num_read)) < 0) {
			throw ServerException("could not read from server");
		}
		num_read += partial_read;
	}
}

SSL_CTX* Server::ssl_create_context(const char *cert, const char *privkey) {
	
	const SSL_METHOD *method;
	SSL_CTX *ctx;

	// TODO: SSLv3 may be better
	method = SSLv23_server_method();
	ctx = SSL_CTX_new(method);
	if (!ctx) {
		ERR_print_errors_fp(stderr);
		throw ServerException("could not create context");
	}

	if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) != 1) {
		ERR_print_errors_fp(stderr);
		throw ServerException("could not load certificate file");
	}
	if (SSL_CTX_use_certificate_chain_file(ctx, cert) != 1) {
		ERR_print_errors_fp(stderr);
		throw ServerException("could not read certificate chain");
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, privkey, SSL_FILETYPE_PEM) != 1) {
		ERR_print_errors_fp(stderr);
		throw ServerException("could not load private key");
	}
	if (!SSL_CTX_check_private_key(ctx)) {
		throw ServerException("private key does not match certificate");
	}

	return ctx;
}

int Server::create_socket(uint16_t port) {
    int fd; 
    struct sockaddr_in addr;
	int one = 1;
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw ServerException("could not create socket");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one)) < 0)
		throw ServerException("could not setsockopt SO_REUSEADDR");
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw ServerException("could not bind to a socket");
    if (listen(fd, 1) < 0)
		throw ServerException("could not create listen queue");

    return fd; 
}

