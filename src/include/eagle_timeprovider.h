#ifndef EAGLE_TIME_PROVIDER_H
#define EAGLE_TIME_PROVIDER_H
#include <string.h>
#include "eagle_lock.h"
#include "eagle_thread.h"
#include "eagle_autoptr.h"

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

namespace eagle
{
class TimeProvider;
typedef AutoPtr<TimeProvider> TimeProviderPtr;

class TimeProvider : public Thread, public AutoPtrBase
{
public:

    static TimeProvider* getInstance();

    TimeProvider() : _terminate(false),_use_tsc(true),_cpu_cycle(0),_buf_idx(0)
    {
        memset(_t,0,sizeof(_t));
        memset(_tsc,0,sizeof(_tsc));

        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        _t[0] = tv;
        _t[1] = tv;
    }

    ~TimeProvider(); 

    time_t getNow()     {  return _t[_buf_idx].tv_sec; }

    void getNow(timeval * tv);
    

    float cpuMHz();

protected:

    virtual void run();

    static ThreadLock        g_tl;

    static TimeProviderPtr   g_tp;

private:
    void setTsc(timeval& tt);

    void addTimeOffset(timeval& tt,const int &idx);

protected:

    bool    _terminate;

    bool    _use_tsc;

private:
    float           _cpu_cycle; 

    volatile int    _buf_idx;

    timeval         _t[2];

    uint64_t        _tsc[2];  
};
}

#endif
