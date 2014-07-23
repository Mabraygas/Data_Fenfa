#ifndef EAGLE_SOCKET_H
#define EAGLE_SOCKET_H
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <string>
#include <cerrno>
#include <cassert>
#include <sstream>
#include <iostream>
#include <string.h>

#include "eagle_ex.h"

namespace eagle
{

class Socket
{
public:
	
	Socket();

	virtual ~Socket();

	void init(int fd, bool owner, int domain = AF_INET);

	void setOwner(bool owner)  { owner_ = owner; }

	bool getOwner() const {return owner_;}
	
	void setDomain(int domain) { domain_ = domain; }

	void createSocket(int socket_type = SOCK_STREAM, int domain = AF_INET);

	int getfd() const { return sock_fd_; }

	bool isValid() const { return sock_fd_ != INVALID_SOCKET; }

	void close();

	int accept(Socket &sock, struct sockaddr *sock_addr, socklen_t &sock_len);

	static void parseAddr(const std::string &addr, struct in_addr &st_addr);

	int setSockOpt(int opt, const void *pvOptVal, socklen_t optLen, int level = SOL_SOCKET);

	void setKeepAlive();

	void setTcpNoDelay();

    int getSockOpt(int opt, void *pvOptVal, socklen_t &optLen, int level = SOL_SOCKET);


	void bind(struct sockaddr *my_addr, socklen_t addrlen);

	void bind(const std::string &server_addr, uint16_t port);

	void connect(const std::string &server_addr, uint16_t port);

	int connectNoThrow(const std::string &server_addr, uint16_t port);

	void listen(int conn_backlog);
	
	int  recv(void *buf, size_t len, int flag = 0);

	int  send(const void *buf, size_t len, int flag = 0);

	void shutdown(int how);

	static void setblock(int fd, bool block);

	void setblock(bool block = false);


private:

	Socket(const Socket &sock);

	Socket& operator=(const Socket &sock);

protected:

	int connect(struct sockaddr *server_addr, socklen_t server_len);

protected:

	static const int INVALID_SOCKET = -1;

	int  sock_fd_;

	bool owner_;

	int  domain_;
	
};

}

#endif
