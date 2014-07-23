// MemManger.h: interface for the CMemManger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ZS_MEMMANGER_H__9F91EE60_F1B3_46D8_B7F2_678C88A0F6A0__INCLUDED_)
#define ZS_MEMMANGER_H__9F91EE60_F1B3_46D8_B7F2_678C88A0F6A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define BIG_MEM_FIELD_SIZE		(1<<10)              //����ڴ�ռ䣬ÿ������
#define SUIT_MEM_FIELD_SIZE	    200                  //С���ڴ�
#define BIG					    0                    //�ڴ�����Ϊ 
#define SUITE				    1                    //�ڴ�����Ϊ

#include "Eps_WIN32_LINUX.h"
class CMemManger  
{
private:
	//�����ڴ�����ռ�
	char *m_BigBuf;
	//С����ڴ�����ռ�
	char *m_SuitBuf;
	//�����ڴ��־
	char** m_BigSymbole;
	//С����ڴ��־
	char** m_SuitSymbole;
	//����С����ڴ����
	long m_SuitFieldNum;
	//��������ڴ����
	long m_BigFieldNum;
	//�ڴ���������־
	CRITICAL_SECTION m_MemCri;
	//
	int m_UsedMemNum[2];
public:

	//��ʼ���ڴ�
	int InitSys(int BigSize,int SuitSize);
	//���������ڴ�ռ�
	int Destory();
	//�����ڴ��С�Ϳռ䳤��
	char* Reqmem(int Mtype,int& Msize);
	//�ͷ�������ڴ�ռ�
	int Freemem(int Mtype,char * Mmem,int MemSize);

public:

	CMemManger();
	virtual ~CMemManger();

};

#endif
