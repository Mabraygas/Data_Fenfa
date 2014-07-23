// MemManger.cpp: implementation of the CMemManger class.
//
//////////////////////////////////////////////////////////////////////

#include "MemManger.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemManger::CMemManger()
{

    InitializeCriticalSection(&m_MemCri);

	m_BigBuf = 0;
	m_SuitBuf = 0;
	
}

CMemManger::~CMemManger()
{
    DeleteCriticalSection(&m_MemCri);	
}
int CMemManger::InitSys(int BigSize,int SuitSize)
{
	    
	if (BigSize >0)
		m_BigBuf = new char[BigSize*BIG_MEM_FIELD_SIZE];
	else
		m_BigBuf = 0;
	
	if (SuitSize >0)
		m_SuitBuf = new char[SuitSize*SUIT_MEM_FIELD_SIZE];
	else
		m_SuitBuf = 0;
	
	m_SuitSymbole = new char*[SuitSize];
    m_BigSymbole =new char*[BigSize];
	m_BigFieldNum = BigSize;
	m_SuitFieldNum = SuitSize;
	
	memset(m_SuitSymbole,0,sizeof(char*)*SuitSize);
	memset(m_BigSymbole,0,sizeof(char*)*BigSize);
	memset(m_UsedMemNum,0,sizeof(int)*2);
	
	return 0;
}

int CMemManger::Destory()
{
	if (m_BigSymbole)
		delete m_BigSymbole;
	
	if (m_BigBuf)
		delete m_BigBuf;
	
	if (m_SuitSymbole)
		delete m_SuitSymbole;
	
	if (m_SuitBuf)
		delete m_SuitBuf;
	
	return 0;
}
char* CMemManger::Reqmem(int Mtype,int& Msize)
{
//	int Msize = Msize1; 
	Mtype = Mtype%2;
	
#ifdef _DEBUG
	if (Mtype == BIG && m_BigBuf)
	{
	    //Msize = (Msize+BIGMEMFIELDSIZE-1)/BIGMEMFIELDSIZE;
		assert(Msize < m_BigFieldNum);
	}
	else 
	{
		//Msize = (Msize+SUITMEMFIELDSIZE-1)/SUITMEMFIELDSIZE;
		assert(Msize < m_SuitFieldNum);
	}
#endif
	
	char *Result =0;
	int i =0;
	int j =0;
	int Temp;
	

	EnterCriticalSection(&m_MemCri);
	
	
	if (Mtype ==BIG && m_BigBuf)
	{
		if ( (m_BigFieldNum - m_UsedMemNum[BIG] ) < Msize) 
		{

			LeaveCriticalSection(&m_MemCri);
			
			return NULL;
		}
		
		for(i=0; i < m_BigFieldNum;)
		{
			if (i+Msize >m_BigFieldNum ) goto PREND;
			
			if ( NULL == m_BigSymbole[i]  && NULL == m_BigSymbole[i + Msize -1])
			{	
				Temp =i;
				for (j = i; j < i+Msize; j++)
				{
					if (m_BigSymbole[j] != NULL)
					{
						i = j+1;
						goto NOMEM;
					}					  
				}
				Result = m_BigBuf+ Temp*BIG_MEM_FIELD_SIZE;
				m_UsedMemNum[BIG] += Msize;
				for ( j = Temp; j < Temp+Msize; j++)
					m_BigSymbole[j] = Result;
				goto PREND;
NOMEM:
				Temp =i;	
			}	
			else
			{
				if (NULL != m_BigSymbole[i] &&  NULL != m_BigSymbole[i + Msize -1])
					i += Msize;
				else
					i ++;
			}
		}
	}
	/////////////////////////////////////////////////////////////
	if (Mtype == SUITE&&m_SuitBuf)
	{
		
		if ( ( m_SuitFieldNum- m_UsedMemNum[SUITE]) < Msize) 
		{
			LeaveCriticalSection(&m_MemCri);			
			//printf("[%d] [%ld]!\n",m_UsedMemNum[SUITE],m_SuitFieldNum-m_UsedMemNum[SUITE]);
			return NULL;
		}
		
		for(i=0; i < m_SuitFieldNum;)
		{
			if (i+Msize >m_SuitFieldNum ) goto PREND;
			if ( NULL == m_SuitSymbole[i]  && NULL == m_SuitSymbole[i + Msize -1])
			{	
				Temp =i;
				for (j = Temp; j < Temp+Msize; j++)
				{
					if (m_SuitSymbole[j] != NULL)
					{
						i = j+1;
						goto NOMEM1;
					}					  
				}
				m_UsedMemNum[SUITE] += Msize;
				Result = m_SuitBuf+ Temp*SUIT_MEM_FIELD_SIZE;		
				for ( j = Temp; j < Temp+Msize; j++)
					m_SuitSymbole[j] = Result;		
				goto PREND;		
NOMEM1:
				Temp =i;
			}	
			else
			{
				if (NULL != m_SuitSymbole[i] &&  NULL != m_SuitSymbole[i + Msize -1])
					i += Msize;
				else
					i ++;
			}
		}
	}	
PREND:
	LeaveCriticalSection(&m_MemCri);

	return Result;
}
int CMemManger::Freemem(int Mtype,char * Mmem,int MemSize)
{
	int i =0;
	if (NULL== Mmem)  return 0;
	int FreeNum =0;
	
    EnterCriticalSection(&m_MemCri);
	if (Mtype==BIG)
	{	
		for (i=(Mmem-m_BigBuf)/BIG_MEM_FIELD_SIZE;i< m_BigFieldNum;i++)
		{
			if ( m_BigSymbole[i] == Mmem)
			{
				m_BigSymbole[i] = NULL;
				FreeNum++;
			}
			else
				break;
		}
		m_UsedMemNum[BIG] -= FreeNum;
	}
	else
	{
		
		for ( i =((Mmem-m_SuitBuf)/SUIT_MEM_FIELD_SIZE);i<m_SuitFieldNum;i++)
		{
			if ( m_SuitSymbole[i] == Mmem)
			{
				m_SuitSymbole[i] = NULL;
				FreeNum++;
			}
			else
				break;
			
		}
		m_UsedMemNum[SUITE] -= FreeNum;
		
	}
	

	LeaveCriticalSection(&m_MemCri);

	if(FreeNum ==0)
		return -1;
    return 0;
}
