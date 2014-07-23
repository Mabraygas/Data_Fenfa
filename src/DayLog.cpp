// DayLog.cpp: implementation of the CDayLog class.
// written by Yuantu
// Date : 2012年5月7日
//////////////////////////////////////////////////////////////////////

#include "DayLog.h"
#include <time.h>
#include <stdarg.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DayLog::DayLog() : tmYear_(0), tmMonth_(0), tmDay_(0), iDays_(0), fp(NULL)
{
    InitializeCriticalSection(&m_Sec);
}

DayLog::~DayLog() {
    DeleteCriticalSection(&m_Sec);
    if(fp) {
        fclose(fp);
    }
}

int DayLog::Init(int iDays, const char * path) {
   if (strlen(path) > 200) {
       return  -1;
   }

   sprintf(m_Path, "%s", path);
   iDays_ = iDays;

   struct timeval  stTmv;
   struct tm       stTm;
   gettimeofday(&stTmv, NULL);
   stTm = *localtime(&stTmv.tv_sec);

   char szLogName[300];
   sprintf(szLogName, "%s_%04d-%02d-%02d.log", m_Path, stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday);
   fp = fopen(szLogName, "a+t");
   if (! fp) {
       return  -2;
   }
   this->tmYear_  = stTm.tm_year;
   this->tmMonth_ = stTm.tm_mon;
   this->tmDay_   = stTm.tm_mday;
   return  0;
}


int DayLog::TWrite(const char * format ...) {

   EnterCriticalSection(&m_Sec);
   if (! fp) {
	   LeaveCriticalSection(&m_Sec);
       return  -1;
   }

   va_list ap;
   va_start(ap, format);
   int ret;


   struct timeval  stTmv;
   struct tm       stTm;
   gettimeofday(&stTmv, NULL);
   stTm = *localtime(&stTmv.tv_sec);

   // 如果 day 变化则切至新一天
   if (stTm.tm_mday != this->tmDay_ || stTm.tm_mon != this->tmMonth_ || stTm.tm_year != this->tmYear_) {
       fclose(fp);
       fp = NULL;
       char szLogName[300];
       sprintf(szLogName, "%s_%04d-%02d-%02d.log", m_Path, stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday);
       fp = fopen(szLogName, "a+t");
       if (! fp)
       {
		   LeaveCriticalSection(&m_Sec);
           return  -2;
	   }
	   // 切到新文件成功
	   this->tmYear_  = stTm.tm_year;
	   this->tmMonth_ = stTm.tm_mon;
	   this->tmDay_   = stTm.tm_mday;
	}

    char  szTm[256];
    sprintf(szTm, "%04d%02d%02d %02d:%02d:%02d",stTm.tm_year + 1900, stTm.tm_mon + 1, stTm.tm_mday,
           stTm.tm_hour, stTm.tm_min, stTm.tm_sec);
    fprintf(fp, "%s.%.6ld ", szTm, stTmv.tv_usec);
    ret = vfprintf(fp, format, ap);
    fflush(fp);

    va_end(ap);

    LeaveCriticalSection(&m_Sec);
    return  ret;
}
