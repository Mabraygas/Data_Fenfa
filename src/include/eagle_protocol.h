#ifndef EAGLE_PROTOCOL_H
#define EAGLE_PROTOCOL_H
#include <string>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <arpa/inet.h>

#include "eagle_epoll_server.h"

namespace eagle
{
struct EagleProtocol
{
public:

	int64_t id;

	uint16_t version;

	char who[16];

	uint32_t key;

	uint32_t body_len;

public:

	const static size_t LEN = sizeof(int64_t)+sizeof(uint16_t)+16+sizeof(uint32_t)+sizeof(uint32_t);

public:

	static std::string WHO;

	static uint32_t KEY;

	static uint16_t VERSION;

public:

	static void BufferToHead(const std::string &buffer, EagleProtocol &head);
	static void BufferToHead(const char *buffer, size_t buflen, EagleProtocol &head)
	{BufferToHead(std::string(buffer,buflen),head);}

	static void HeadToBuffer(const EagleProtocol &head, std::string &buffer);

public:

	EagleProtocol();

	EagleProtocol(const EagleProtocol &head);

	EagleProtocol &operator=(const EagleProtocol &head);

public:

	static int ParseProtocol(std::string &buffer, std::string &o);

private:

	void copy(const EagleProtocol &head);
};

}

#endif

