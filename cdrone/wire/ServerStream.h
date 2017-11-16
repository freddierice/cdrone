#ifndef __SERVER_H__
#define __SERVER_H__
#include <inttypes.h>

#include <stdexcept>
#include <string>

#include <google/protobuf/io/zero_copy_stream.h>
#include <openssl/ssl.h>

// A ZeroCopyInputStream which reads from an SSL socket.
class SSLInputStream : public google::protobuf::io::ZeroCopyInputStream {
public:
	// Creates a stream that reads from the given SSL handle. If a block_size
	// is given, it specifies the number of bytes that should be read and 
	// returned with each call to Next(). Otherwise, a reasonable default is 
	// used.
	explicit SSLInputStream(SSL *ssl, int block_size = -1);
	
	virtual bool Next(const void **data, int *size);
	virtual void Backup(int count);
	virtual bool Skip(int count);
	virtual google::protobuf::int64 ByteCount() const;
private:
	SSL *m_ssl;
};

class ServerOutputStream : google::protobuf::io::ZeroCopyOutputStream {

};

#endif
