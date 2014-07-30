// Global.h: interface for the Global class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBAL_H__4E7168E5_1269_48CE_9D2E_E03BE0B5BF76__INCLUDED_)
#define AFX_GLOBAL_H__4E7168E5_1269_48CE_9D2E_E03BE0B5BF76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Eps_WIN32_LINUX.h"

class QueEvent
{
public:
	QueEvent(int lQueSize)
	{
		InitializeCriticalSection(&cri);
		InitializeCriticalSection(&cri1);
		lSize = lQueSize;
		lpQue = new int[lQueSize];
		lBegin = 0;
		lEnd = 0;
		lNum = 0;
#ifdef WIN32
		char tmp[100];
		//sprintf(tmp,"qzbjl%epsindexsearch",this);
		sprintf(tmp,"qzbjl%dpsindexsearch",time(NULL));
		h = CreateEvent(NULL,TRUE,FALSE,tmp);	
#endif
		
	};
	~QueEvent()
	{
		DeleteCriticalSection(&cri);
		delete lpQue;
#ifdef WIN32
		::CloseHandle(h);
#endif
	};
	int GetFreeNum()
	{
		int lRet ;
		EnterCriticalSection(&cri);
		lRet = lSize - lNum;
		LeaveCriticalSection(&cri);
		return lRet;

	}
	int Push_W(int lValue)
	{
		while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum >= lSize)
			{
				LeaveCriticalSection(&cri);
				//printf("Global sleep Push\n");
				Sleep(1);
				continue;
			}
			lpQue[lEnd ++] = lValue;
			if(lEnd >= lSize)
				lEnd = 0;
			lNum ++;
#ifdef WIN32
			SetEvent(h);
#endif
			
			LeaveCriticalSection(&cri);
			return 0;
		}
	};
	int Push(int lValue)
	{
		while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum >= lSize)
			{
				LeaveCriticalSection(&cri);
				Sleep(1);
				continue;
				//return -1;
			}
			lpQue[lEnd ++] = lValue;
			if(lEnd >= lSize)
				lEnd = 0;
			lNum ++;
#ifdef WIN32
			SetEvent(h);
#endif
			
			LeaveCriticalSection(&cri);
			return 0;
		}
	};
	int PushEvent(int lValue)
	{
		while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum >= lSize)
			{
				LeaveCriticalSection(&cri);
				Sleep(1);
				continue;
			}
			lpQue[lEnd ++] = lValue;
			if(lEnd >= lSize)
				lEnd = 0;
			lNum ++;
			
#ifdef WIN32
		if(lNum == 1)
				SetEvent(h);
#endif
		
			LeaveCriticalSection(&cri);
			return 0;
		}
	};
	int Pop_W()
	{
		long lRet = 0;
		while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum <= 0)
			{
				LeaveCriticalSection(&cri);
				Sleep(1);
				continue;
			}
			lRet = lpQue[lBegin ++];
			if(lBegin >= lSize)
				lBegin = 0;
			lNum --;
			LeaveCriticalSection(&cri);
			return lRet;
		}
		return lRet;
	};
	int Pop()
	{
		int lRet = 0;
        while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum <= 0)
			{
				LeaveCriticalSection(&cri);
				//Sleep(1);
                return -1 ;
			}
			lRet = lpQue[lBegin ++];
			if(lBegin >= lSize)
				lBegin = 0;
			lNum --;
			LeaveCriticalSection(&cri);
			return lRet;
		}
		return lRet;
	};
	int PopAll(int *pRet)
	{
		long lRet = 0;
		while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum <= 0)
			{
				LeaveCriticalSection(&cri);
				//Sleep(1);
				return -1;
			}
			lRet = lNum;
			for (int i = 0; i < lRet; i ++)
			{
				pRet[i] = lpQue[lBegin ++];
				if(lBegin >= lSize)
					lBegin = 0;
				lNum --;
			}
			LeaveCriticalSection(&cri);
			return lRet;
		}
		return lRet;
	};
	int PopEvent()
	{
		long lRet = 0;
		EnterCriticalSection(&cri1);
#ifdef WIN32
		WaitForSingleObject(h,INFINITE);
#endif
		while(1)
		{
			EnterCriticalSection(&cri);
			if(lNum <= 0)
			{
				LeaveCriticalSection(&cri);
				Sleep(1);
				continue;
				
			}
			lRet = lpQue[lBegin ++];
			if(lBegin >= lSize)
				lBegin = 0;
			lNum --;
#ifdef WIN32
			if(lNum <= 0)
				ResetEvent(h);
#endif
		
			LeaveCriticalSection(&cri);
			LeaveCriticalSection(&cri1);
			return lRet;
		}
		LeaveCriticalSection(&cri1);
		return lRet;
	};
	
	int lNum;
	int lBegin;
	int lEnd;
	int lSize;
	int *lpQue;
	
#ifdef WIN32
	HANDLE h;
#endif

	CRITICAL_SECTION cri;
	CRITICAL_SECTION cri1;
	
};

#endif // !defined(AFX_GLOBAL_H__4E7168E5_1269_48CE_9D2E_E03BE0B5BF76__INCLUDED_)
