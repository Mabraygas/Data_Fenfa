#ifndef EAGLE_THREAD_QUEUE_H
#define EAGLE_THREAD_QUEUE_H

#include <deque>
#include "eagle_lock.h"

namespace eagle
{
template<typename T, typename D = std::deque<T> >
class ThreadQueue : public ThreadLock
{
public:

	typedef D queue_type;

public:

	ThreadQueue() : size_(0)
	{}

	bool pop_front(T &t, int millsecond = 0);

	void push_back(const T &t);

	void push_back(const queue_type &qt);

	void push_front(const T &t);

	void push_front(const queue_type &qt);

	bool swap(queue_type &qt, int millsecond = 0);

	void swap_(queue_type &qt);

	void notifyT();

	size_t size() const;

	void clear();

	bool empty() const;

protected:

	queue_type queue_;

	size_t size_;
};

template<typename T, typename D>
bool ThreadQueue<T, D>::pop_front(T &t, int millsecond)
{
	Lock lock(*this);

	if(queue_.empty()){
		if(0 == millsecond){
			return false;
		}
		if(-1 == millsecond){
			wait();
		}
		else{
			if(!timedWait(millsecond)){
				return false;
			}
		}
	}

	if(queue_.empty()){
		return false;
	}

	t = queue_.front();
	queue_.pop_front();
	
	if(0 >= size_){
		throw EagleException("[ThreadQueue::pop_front] size_ <= 0");
	}
	--size_;

	return true;
}


template<typename T, typename D>
void ThreadQueue<T, D>::push_back(const T &t)
{
	Lock lock(*this);

	notify();

	queue_.push_back(t);
	++size_;
}

template<typename T, typename D>
void ThreadQueue<T, D>::push_back(const queue_type &qt)
{
	Lock lock(*this);

	typename queue_type::const_iterator it = qt.begin();
	typename queue_type::const_iterator itEnd = qt.end();
	while(it != itEnd){
		queue_.push_back(*it);
		++it;
		++size_;

		notify();
	}
}

template<typename T, typename D>
void ThreadQueue<T, D>::push_front(const T &t)
{
	Lock lock(*this);

	notify();

	queue_.push_front(t);
	++size_;
}

template<typename T, typename D>
void ThreadQueue<T, D>::push_front(const queue_type &qt)
{
	Lock lock(*this);

	typename queue_type::const_iterator it = qt.begin();
	typename queue_type::const_iterator itEnd = qt.end();
	while(it != itEnd){
		queue_.push_front(*it);
		++it;
		++size_;

		notify();
	}
}

template<typename T, typename D>
bool ThreadQueue<T, D>::swap(queue_type &qt, int millsecond)
{
	Lock lock(*this);

	if(queue_.empty()){
		if(0 == millsecond){
			return false;
		}
		if(-1 == millsecond){
			wait();
		}
		else{
			if(!timedWait(millsecond)){ 
				return false;
			}
		}
	}

	if(queue_.empty()){
		return false;
	}

	size_ = qt.size();
	qt.swap(queue_);

	return true;
}

template<typename T, typename D>
void ThreadQueue<T, D>::swap_(queue_type &qt)
{
	Lock lock(*this);

	size_ = qt.size();
	qt.swap(queue_);
}

template<typename T, typename D>
void ThreadQueue<T, D>::notifyT()
{
	Lock lock(*this);

	notifyAll();
}

template<typename T, typename D>
size_t ThreadQueue<T, D>::size() const
{
	Lock lock(*this);
	return size_;
}

template<typename T, typename D>
void ThreadQueue<T, D>::clear()
{
	Lock lock(*this);

	queue_.clear();
	size_ = 0;
}

template<typename T, typename D>
bool ThreadQueue<T, D>::empty() const
{
	Lock lock(*this);

	return queue_.empty();
}

}


#endif
