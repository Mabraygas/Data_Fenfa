// DayLog.h: interface for the CDayLog class.
// written by Yuantu
// Date : 2012��5��7��
//////////////////////////////////////////////////////////////////////
#ifndef __DAYLOG_H__
#define __DAYLOG_H__

#include "Eps_WIN32_LINUX.h"

class DayLog {
    int     tmYear_;
    int     tmMonth_;
    int     tmDay_;

    int     iDays_; // ֻ������Щ�����־��δʵ�ֻ���

    char    m_Path[300];
    CRITICAL_SECTION m_Sec;
    FILE    * fp;
public:
    time_t  tmLast_;

    // ��ʼ�����������ļ��ȣ�
    // $iDays : ֻ���� $iDays ����־��δʵ�ֻ���
    // $path  : ��־�ļ�·�������Ϊ "../log/CutServer" ����־��Ϊ "../log/CutServer_2011-05-31.log" ����ʽ
    int     Init(int iDays, const char * path);

    int TWrite(const char * format ...);

    DayLog();
    ~DayLog();
};
#endif
