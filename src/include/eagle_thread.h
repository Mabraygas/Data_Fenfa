#ifndef EAGLE_THREAD_H
#define EAGLE_THREAD_H
#include <time.h>
#include <pthread.h>
#include "eagle_ex.h"
#include "eagle_lock.h"

namespace eagle
{
class ThreadControl
{
public:

	ThreadControl();

	explicit ThreadControl(pthread_t thread);

	void join();

	void detach();

	pthread_t id() const;

	static void sleep(long millsecond);

	static void yield();
protected:

	pthread_t thread_;
};


class Runable
{
public:
	virtual ~Runable()
	{}

	virtual void run() = 0;
};

class Thread : public Runable
{
public:

	Thread();

	virtual ~Thread(){};

	ThreadControl start();

	ThreadControl getThreadControl() const 
	{ return ThreadControl(tid_); }

	bool isAlive() const { return running_; }

	pthread_t id() { return tid_; }

protected:

	static void threadEntry(Thread *pThread);

	virtual void run() = 0;

protected:

	bool running_;

	pthread_t tid_;

	ThreadLock lock_;
};

}

#endif
