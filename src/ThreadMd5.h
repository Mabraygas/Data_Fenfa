#ifndef _THREAD_MD5_H_
#define _THREAD_MD5_H_

#include <sys/types.h>

class ThreadMd5
{
	public:
		ThreadMd5();
		~ThreadMd5();
		void join();
		void start();
		int run();
		static void* callback(void* lpvoid);
		int buildMd5(const char* pFile);
	private:
		pthread_t m_tid;
		char*     m_pBuf;
};

#endif
