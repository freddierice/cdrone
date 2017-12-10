#ifndef __SERVER_H__
#define __SERVER_H__
#include <stdexcept>
#include <string>
#include <utility>

#include <inttypes.h>

#include "misc/Config.h"
#include "misc/exception.h"

#include <google/protobuf/io/zero_copy_stream.h>

#include <openssl/ssl.h>

CDRONE_EXCEPTION(CDroneException, ServerException);

class Server {
public:
	Server(Config &config);
	~Server();

	std::pair<google::protobuf::io::ZeroCopyInputStream*,
		google::protobuf::io::ZeroCopyOutputStream*> accept();
	void reset();

private:
	Server();
	static SSL_CTX* ssl_create_context(const char *cert, const char *privkey);
	static int create_socket(uint16_t port);
	void init();
	void cleanup();

	SSL_CTX* m_ctx;
	std::string m_certificate, m_key;
	int m_server_fd;
	uint16_t m_port;
	bool m_use_ssl;
};

#endif
