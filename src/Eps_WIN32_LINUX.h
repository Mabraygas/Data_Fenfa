// Eps_WIN32_LINUX.h: interface for the Eps_WIN32_LINUX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _EPS_LINUX_H__
#define _EPS_LINUX_H__
//#define _LINUX_
//#define _X64_ //64位操作系统的定义
#define _PSIZE_ sizeof(char*) //指针长度 方便计算偏移地址

//通信协议中需要用到的功能

#define FUN_DF            0xDDFF0000          //通信协议的功能
#define FUN_DF_CREATELIST 0xDDFF0001          //创建list文件
#define FUN_DF_CLOSELIST  0xDDFF0002          //关闭list文件
#define FUN_DF_OPENAFILE  0xDDFF0003          //打开一个文件
#define FUN_DF_CLOSEAFILE 0xDDFF0004          //关闭一个文件
#define FUN_DF_WRITEAFILE 0xDDFF0005          //往文件中写数据

#define RET_DF_SUCCESS    0XDDFF0006          //返回成功
#define RET_DF_FAILD      0XDDFF0007          //返回失败

#define DF_RET_LEN        12                  //返回包长度 12Bytes


#ifdef WIN32 
 #if _MSC_VER > 1000
 #pragma once
 #endif // _MSC_VER > 1000
 #include <windows.h>

 	 
#else
    #include <time.h>
	#include <cerrno>
	#include <cstring>
	#include <cstdlib>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/time.h>
    #include <sys/times.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
	#include <fcntl.h>
	#include <string.h>
	#include <curses.h>
    #include <sys/epoll.h>

	#define CRITICAL_SECTION			pthread_mutex_t 
    #define __int64						long long
	#define __int32						int
	#define __int16						short
	#define TryEnterCriticalSection		pthread_mutex_trylock	
	#define EnterCriticalSection		pthread_mutex_lock
	#define LeaveCriticalSection		pthread_mutex_unlock
	#define DeleteCriticalSection		pthread_mutex_destroy
	#define InitializeCriticalSection	Eps_WIN32_LINUX::InitCri
	#define _atoi64						Eps_WIN32_LINUX::_atoi64_Eps   
    #define _MT
	#define SOCKET						int
	#define	SOCKADDR_IN					sockaddr_in
	#define INVALID_SOCKET				(SOCKET)(~0)
	#define Sleep						Eps_WIN32_LINUX::_Sleep_Eps
	#define closesocket					close
	#define	LPVOID						void*
	#define filelength					Eps_WIN32_LINUX::Eps_filelength 
	#define GetTickCount				Eps_WIN32_LINUX::Eps_GetTickCount
    #define DWORD						unsigned long 
	//#define SLEEPMSECOND 
#endif

#ifndef WIN32
   // 输入时间的 second 数量，返回两个时间的自然天数差，如果两个时间都处于同一日，则为 0 天
// 如果 $tmPast > $tmNow 则返回 0
// $tmPast, $tmNow 分别是较早的时间和较近的时间的秒数
  inline long GetNatureDayDiffSec(time_t tmPast, time_t tmNow = 0) {
	if (0 == tmNow) {
		tmNow = time(NULL);
	}
	if (tmPast >= tmNow) {
		return  0;
	}
//	struct tm  stTmNow, stTmPast;
//	stTmNow  = *localtime(&tmNow);
//	stTmPast = *localtime(&tmPast);
//	stTmNow.tm_hour = 0;
//	stTmNow.tm_min  = 0;
//	stTmNow.tm_sec  = 0;
//	tmNow = mktime(&stTmNow);
//	stTmPast.tm_hour = 0;
//	stTmPast.tm_min  = 0;
//	stTmPast.tm_sec  = 0;
//	tmPast = mktime(&stTmPast);
	if (tmNow < 0 || tmPast < 0) {
		return  0;
	}
	return  (tmNow - tmPast) / 86400;
}
#endif

//usleep
class Eps_WIN32_LINUX  
{
public:
   static int InitCri(CRITICAL_SECTION* Sec,void* Para =NULL )
	{
#ifdef WIN32
		InitializeCriticalSection(Sec);
#else
		
		return pthread_mutex_init(Sec, NULL);
#endif	
		return 0;
	   
	};
#ifndef WIN32
   static  long long _atoi64_Eps(char *Str)
   {
	   return strtoll(Str,NULL,10);
   };
   //毫秒的暂停函数
   static void _Sleep_Eps(int MSecond)
   {
#ifdef SLEEPMSECOND
		usleep(MSecond);
#else
		usleep(MSecond*1000);
#endif
		
   };
   //毫秒的获取函数
   static long Eps_GetTickCount()
   {
		timeval stval;
        gettimeofday(&stval,NULL);
        long lTime = stval.tv_sec*1000+stval.tv_usec/1000;
        return lTime;
	   //tms tm;
	   //return times(&tm);
   };
   //微妙的获取函数
   static long Eps_GetUTickCount()
   {
	   	timeval stval;
        gettimeofday(&stval,NULL);
        long lTime = stval.tv_sec*1000000+stval.tv_usec;
        return lTime;
   }
#endif



public:
#ifndef WIN32
	static long Eps_filelength(int handle)
	{
		return lseek(handle,0,SEEK_END);
	};
#endif

public:
   Eps_WIN32_LINUX(){};
   virtual ~Eps_WIN32_LINUX(){};

};




#endif 
