#ifndef EAGLE_ENDPOINT_H
#define EAGLE_ENDPOINT_H
#include <string>
#include <sstream>
#include <stdint.h>
#include "eagle_ex.h"

namespace eagle
{
class Endpoint
{
public:

    Endpoint();

    Endpoint(const std::string& host, uint16_t port, int timeout, int istcp = true, int grid = 0)
    {
        host_    = host;
        port_    = port;
        timeout_ = timeout;
        istcp_   = istcp;
        grid_    = 0;
    }

    Endpoint(const std::string& desc)
    {
        parse(desc);
    }

    Endpoint(const Endpoint& l)
    {
        host_   = l.host_;
        port_   = l.port_;
        timeout_= l.timeout_;
        istcp_  = l.istcp_;
        grid_   = l.grid_;
    }

    Endpoint& operator = (const Endpoint& l)
    {
        if(this != &l) {
            host_   = l.host_;
            port_   = l.port_;
            timeout_= l.timeout_;
            istcp_  = l.istcp_;
            grid_   = l.grid_;
        }

        return *this;
    }

    bool operator == (const Endpoint& l)
    {
        return (host_    == l.host_ && 		\
				port_    == l.port_ && 		\
				timeout_ == l.timeout_ && 	\
				istcp_   == l.istcp_ && 	\
				grid_    == l.grid_);
    }

    void setHost(const std::string& host) 
	{ host_ = host; }

	std::string getHost() const 
	{ return host_; }

    void setPort(uint16_t port)              
	{ port_ = port; }

    uint16_t getPort() const                 
	{ return port_; }

    void setTimeout(int timeout)        
	{ timeout_ = timeout; }

    int getTimeout() const              
	{ return timeout_; }

    bool isTcp() const                  
	{ return istcp_; }

    void setTcp(bool istcp)              
	{ istcp_ = istcp; }

    int getGrid() const                 
	{ return grid_; }

    void setGrid(int grid)              
	{ grid_ = grid; }

    bool IsUnixLocal() const            
	{ return port_ == 0; }

	std::string ToString() const
    {
		std::ostringstream os;
        os << (isTcp()?"tcp" : "udp") << " -h " << host_ << " -p " << port_ << " -t " << timeout_;
        if (grid_ != 0) os << " -g " << grid_;
        return os.str();
    }

    void parse(const std::string &desc);

protected:

	std::string host_;

	uint16_t port_;

	int timeout_;

	bool istcp_;

	int grid_;
};

}

#endif
