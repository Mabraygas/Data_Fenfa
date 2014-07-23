// MemManger.h: interface for the CMemManger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ZS_MEMMANGER_H__9F91EE60_F1B3_46D8_B7F2_678C88A0F6A0__INCLUDED_)
#define ZS_MEMMANGER_H__9F91EE60_F1B3_46D8_B7F2_678C88A0F6A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define BIG_MEM_FIELD_SIZE		(1<<10)              //大块内存空间，每次申请
#define SUIT_MEM_FIELD_SIZE	    200                  //小块内存
#define BIG					    0                    //内存类型为 
#define SUITE				    1                    //内存类型为

#include "Eps_WIN32_LINUX.h"
class CMemManger  
{
private:
	//大块的内存申请空间
	char *m_BigBuf;
	//小块的内存申请空间
	char *m_SuitBuf;
	//大块的内存标志
	char** m_BigSymbole;
	//小块的内存标志
	char** m_SuitSymbole;
	//申请小块的内存个数
	long m_SuitFieldNum;
	//申请大块的内存个数
	long m_BigFieldNum;
	//内存申请锁标志
	CRITICAL_SECTION m_MemCri;
	//
	int m_UsedMemNum[2];
public:

	//初始化内存
	int InitSys(int BigSize,int SuitSize);
	//销毁整个内存空间
	int Destory();
	//申请内存大小和空间长度
	char* Reqmem(int Mtype,int& Msize);
	//释放申请的内存空间
	int Freemem(int Mtype,char * Mmem,int MemSize);

public:

	CMemManger();
	virtual ~CMemManger();

};

#endif
