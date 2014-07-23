// QSEPSDF_SS.h: interface for the CQSEPSDF_SS class.
/**
 * @file QSEPSDF_SS.h
 * @brief 服务器入口
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-10-11
 */

#ifndef _QSEPSDF_SS_H
#define _QSEPSDF_SS_H

#include "Global.h"
#include "Eps_WIN32_LINUX.h"
#include "ThreadMd5.h"

using namespace std;

class CQSEPSDF_SS  
{
public:
	CQSEPSDF_SS();
	virtual ~CQSEPSDF_SS();

public:
	int InitSys(uint16_t port, uint32_t threadnum);
	int StartServer();
private:
	uint16_t port;
	uint32_t threadnum;
	//ThreadMd5*  m_pthreadMd5;

};

#endif //QSEPSDF_SS
