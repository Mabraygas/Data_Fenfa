#ifndef EAGLE_LOCK_H
#define EAGLE_LOCK_H

#include <time.h>
#include <string>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>

#include "eagle_ex.h"

namespace eagle
{

template <typename T>
class LockT
{
public:

	LockT(const T& mutex) : mutex_(mutex)
	{
		mutex_.lock();
		acquired_ = true;
	}

	virtual ~LockT()
    {
        if (acquired_) {
            mutex_.unlock();
        }
    }

	void acquire() const
	{
		if (acquired_) {
            throw EagleException("thread has locked!");
        }
        mutex_.lock();
        acquired_ = true;
	}

	bool tryAcquire() const
    {
        acquired_ = mutex_.tryLock();
        return acquired_;
    }

	void release() const
	{
		if (!acquired_) {
            throw EagleException("thread hasn't been locked!");
        }
        mutex_.unlock();
        acquired_ = false;
	}

	bool acquired() const
    {
        return acquired_;
    }

protected:

	LockT(const LockT&);

	LockT& operator=(const LockT&);

protected:

	const T& mutex_;

	mutable bool acquired_;
};


class ThreadCond; 

class ThreadMutex
{
public:

	typedef LockT<ThreadMutex> Lock;

public:

	ThreadMutex();

	virtual ~ThreadMutex();

	void lock() const;

	bool tryLock() const;

	void unlock() const;

protected:

	ThreadMutex(const ThreadMutex&);

	void operator=(const ThreadMutex&);

	friend class ThreadCond;

protected:

	mutable pthread_mutex_t mutex_;
};

class ThreadCond
{
public:

	ThreadCond();

	~ThreadCond();

	void signal();

	void broadcast();

	template<typename Mutex>
    void wait(const Mutex& mutex) const
    {   
        int rc = pthread_cond_wait(&cond_, &mutex.mutex_);
        if(rc != 0) {   
            throw EagleException("[ThreadCond::wait] pthread_cond_wait error", errno);
        }   
    }   

	template<typename Mutex>
    bool timedWait(const Mutex& mutex, int millsecond) const
    {
		struct timeval tv;
		gettimeofday(&tv, 0);
		int64_t it  = tv.tv_sec * (int64_t)1000000 + tv.tv_usec + millsecond * 1000;
		tv.tv_sec   = it / (int64_t)1000000;
		tv.tv_usec  = it % (int64_t)1000000; 

		timespec ts;
		ts.tv_sec   = tv.tv_sec;
		ts.tv_nsec  = tv.tv_usec * 1000; 

        int rc = pthread_cond_timedwait(&cond_, &mutex.mutex_, &ts);
        if(rc != 0) {
            if(rc != ETIMEDOUT) {
                throw EagleException("[ThreadCond::timedWait] pthread_cond_timedwait error", errno);
            }

            return false;
        }
        return true;
	}

protected:
	
    ThreadCond(const ThreadCond&);

    ThreadCond& operator=(const ThreadCond&);

protected:

	mutable pthread_cond_t cond_;
};

template <class T, class P>
class ThreadSurpervisor
{
public:

	typedef LockT<ThreadSurpervisor<T,P> > Lock;

public:

	ThreadSurpervisor() : notify_(0)
	{}

	virtual ~ThreadSurpervisor()
	{}

	void lock() const
	{
		mutex_.lock();
		notify_ = 0; 
	}

	bool tryLock() const
	{
		bool result = mutex_.tryLock();
		if(result){
			notify_ = 0; 
		}

		return result;
	}

	void unlock() const
	{
		notifyImpl(notify_);

		try{
			mutex_.unlock();
		}catch(...){
			notify_ = 0;
			throw;
		}

		notify_ = 0;
	}

	void wait() const
	{
		notifyImpl(notify_);

		try{
			cond_.wait(mutex_);
		}
		catch(...){
			notify_ = 0;
			throw;
		}

		notify_ = 0;
	}

	bool timedWait(int millsecond) const
	{
		notifyImpl(notify_);

		bool rc;
		try{
			rc = cond_.timedWait(mutex_,millsecond);
		}
		catch(...){
			notify_ = 0;
			throw;
		}

		notify_ = 0;
		return rc;
	}

	void notify()
	{
		if(-1 != notify_){
			++notify_;
		}
	}

	void notifyAll()
	{
		notify_ = -1;
	}

protected:

	ThreadSurpervisor(const ThreadSurpervisor&);
	
	ThreadSurpervisor& operator=(const ThreadSurpervisor&);

protected:

	void notifyImpl(int notify) const
	{
		if(0 == notify)
			return;

		if(-1 == notify){
			cond_.broadcast();
			return;
		}
		else{
			while(notify > 0){
				cond_.signal();
				--notify;
			}
		}
	}

protected:

	mutable int notify_;

	mutable P cond_;

	T mutex_;
};



typedef ThreadSurpervisor<ThreadMutex,ThreadCond> ThreadLock;

typedef ThreadMutex MutexLock;


}

#endif


