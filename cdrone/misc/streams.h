#ifndef __STREAMS_H__
#define __STREAMS_H__

// ReaderWriter is an interface for objects that can read and write.
class ReaderWriter {
public:
	virtual int read(void* buffer, int n);
	virtual int write(const void* buffer, int n);
};

// A stream interface for easily reading from streams. I am not using iostream
// because it uses too many buffers.
class Stream {
public:
	void next(const void** buffer, int* n);
	void backUp(int count);
};

class FileDescriptorStream : Stream {
	FileDescriptorStream(int fd);
};

#endif /* __STREAMS_H__ */
