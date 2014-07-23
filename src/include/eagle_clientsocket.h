#ifndef EAGLE_CLIENTSOCKET_H
#define EAGLE_CLIENTSOCKET_H
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <string>
#include <cerrno>
#include <sstream>
#include <sstream>
#include <string.h>

#include "eagle_ex.h"
#include "eagle_socket.h"
#include "eagle_epoller.h"
#include "eagle_protocol.h"

namespace eagle
{

class ClientSocket
{
public:

	ClientSocket():timeout_(3000){}

	ClientSocket(const std::string &ip, int port, int timeout){ init(ip, port, timeout); }

	virtual ~ClientSocket(){}

	void init(const std::string &ip, int port, int timeout)
    {   
		sock_.close();
		ip_   = ip;
		port_ = port;
		timeout_ = timeout;
    }

	virtual int send(const char *buffer, size_t len, std::string &err) = 0;

	virtual int recv(char *buffer, size_t &len, std::string &err) = 0;

protected:
	
	Socket sock_;
	
	std::string ip_;

	int port_;

	int timeout_;
};


class TCPClient : public ClientSocket
{
public:

	TCPClient() : nodelay_(false){}

	TCPClient(const std::string &ip, int port, int timeout=3000, bool nodelay = false) 
		: ClientSocket(ip, port, timeout) 
		, nodelay_(nodelay)
	{}

	void set_nodelay(bool nodelay) {nodelay_ = nodelay;}

	int send(const char *buffer, size_t len, std::string &err);

	int send(const std::string &buffer, std::string &err) {return send(buffer.c_str(),buffer.size(),err);}

	int sendEagleProtocol(const std::string &body, std::string &err);
	int sendEagleProtocol(EagleProtocol &head, const std::string &body, std::string &err);

	int recv(char *buffer, size_t &len, std::string &err);

	int recvLength(char *buffer, size_t len, std::string &err);

	int recvEagleProtocol(EagleProtocol &head, std::string &body, std::string &err);
	int recvEagleProtocol(std::string &body, std::string &err);

	void close() 
	{sock_.close();}

protected:

	int checkSocket(std::string &err);

public:

	const static uint32_t TMPBUF_SIZE = 1024*16;

protected:

	bool nodelay_;
};

}

#endif
