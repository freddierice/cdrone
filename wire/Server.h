#ifndef __SERVER_H__
#define __SERVER_H__
#include <openssl/ssl.h>
#include <inttypes.h>

#include <stdexcept>
#include <string>

class ServerException : public std::runtime_error {
public:
    ServerException(std::string const& msg) : std::runtime_error(msg) {}
};

class Server {
public:
	Server(uint16_t port, const std::string& certificate,
			const std::string& key);
	~Server();

	void accept();
	void reset();

	void readAll(void *buf, int size);

private:
	Server();
	static SSL_CTX* ssl_create_context(const char *cert, const char *privkey);
	static int create_socket(uint16_t port);
	void init();
	void cleanup();

	SSL_CTX* m_ctx;
	SSL *m_ssl;
	std::string m_certificate, m_key;
	int m_server_fd, m_client_fd;
	uint16_t m_port;
};

#endif
