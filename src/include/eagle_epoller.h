#ifndef EAGLE_EPOLLER_H
#define EAGLE_EPOLLER_H
#include <cassert>
#include <unistd.h>
#include <sys/epoll.h>

namespace eagle
{

class Epoller
{
public:
	
	Epoller(bool et = true);	

	~Epoller();

	void create(int max_connections);

	void add(int fd, uint64_t data, __uint32_t event);

	void mod(int fd, uint64_t data, __uint32_t event);

	void del(int fd, uint64_t data, __uint32_t event);

	int wait(int millsecond);

	struct epoll_event& get(int i) { assert(p_evs_ != 0); return p_evs_[i]; }

	int get_epoll_fd()
	{
		return epoll_fd_;
	}

protected:

	void ctrl(int fd, uint64_t data, __uint32_t events, int op);

protected:

	int epoll_fd_;

	int int_max_connections_;

	struct epoll_event *p_evs_;

	bool b_et_;
};

}

#endif
