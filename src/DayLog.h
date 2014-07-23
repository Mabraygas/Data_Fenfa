// DayLog.h: interface for the CDayLog class.
// written by Yuantu
// Date : 2012年5月7日
//////////////////////////////////////////////////////////////////////
#ifndef __DAYLOG_H__
#define __DAYLOG_H__

#include "Eps_WIN32_LINUX.h"

class DayLog {
    int     tmYear_;
    int     tmMonth_;
    int     tmDay_;

    int     iDays_; // 只保留这些天的日志（未实现还）

    char    m_Path[300];
    CRITICAL_SECTION m_Sec;
    FILE    * fp;
public:
    time_t  tmLast_;

    // 初始化，包括打开文件等；
    // $iDays : 只保留 $iDays 的日志（未实现还）
    // $path  : 日志文件路径，如果为 "../log/CutServer" 则日志名为 "../log/CutServer_2011-05-31.log" 的形式
    int     Init(int iDays, const char * path);

    int TWrite(const char * format ...);

    DayLog();
    ~DayLog();
};
#endif
