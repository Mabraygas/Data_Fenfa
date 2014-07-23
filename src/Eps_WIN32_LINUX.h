// Eps_WIN32_LINUX.h: interface for the Eps_WIN32_LINUX class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _EPS_LINUX_H__
#define _EPS_LINUX_H__
//#define _LINUX_
//#define _X64_ //64λ����ϵͳ�Ķ���
#define _PSIZE_ sizeof(char*) //ָ�볤�� �������ƫ�Ƶ�ַ

//ͨ��Э������Ҫ�õ��Ĺ���

#define FUN_DF            0xDDFF0000          //ͨ��Э��Ĺ���
#define FUN_DF_CREATELIST 0xDDFF0001          //����list�ļ�
#define FUN_DF_CLOSELIST  0xDDFF0002          //�ر�list�ļ�
#define FUN_DF_OPENAFILE  0xDDFF0003          //��һ���ļ�
#define FUN_DF_CLOSEAFILE 0xDDFF0004          //�ر�һ���ļ�
#define FUN_DF_WRITEAFILE 0xDDFF0005          //���ļ���д����

#define RET_DF_SUCCESS    0XDDFF0006          //���سɹ�
#define RET_DF_FAILD      0XDDFF0007          //����ʧ��

#define DF_RET_LEN        12                  //���ذ����� 12Bytes


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
   // ����ʱ��� second ��������������ʱ�����Ȼ������������ʱ�䶼����ͬһ�գ���Ϊ 0 ��
// ��� $tmPast > $tmNow �򷵻� 0
// $tmPast, $tmNow �ֱ��ǽ����ʱ��ͽϽ���ʱ�������
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
   //�������ͣ����
   static void _Sleep_Eps(int MSecond)
   {
#ifdef SLEEPMSECOND
		usleep(MSecond);
#else
		usleep(MSecond*1000);
#endif
		
   };
   //����Ļ�ȡ����
   static long Eps_GetTickCount()
   {
		timeval stval;
        gettimeofday(&stval,NULL);
        long lTime = stval.tv_sec*1000+stval.tv_usec/1000;
        return lTime;
	   //tms tm;
	   //return times(&tm);
   };
   //΢��Ļ�ȡ����
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
