#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include<sys/types.h>
#include<sys/stat.h>

#include "ThreadMd5.h"
#include "QSEPSDF_MainControl.h"

#include "Global.h"
#include "md5.h"
#include "DayLog.h"

using namespace std;

#define  LOG(format, args...) g_Srvlog.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

extern DayLog   g_Srvlog;


extern string   g_QueFile[MAX_FILE_NUM];
extern QueEvent g_FilePos;
extern QueEvent g_WorkPos;


ThreadMd5::ThreadMd5()
{
	m_pBuf = new char[MAX_SEND_SIZE];
	m_tid = 0;
}

ThreadMd5::~ThreadMd5()
{
	if(m_pBuf)
		delete [] m_pBuf;
}

void ThreadMd5::join()
{
	if(m_tid)
	{
		pthread_join(m_tid, NULL);
		m_tid = 0;
	}
}

void ThreadMd5::start()
{
	pthread_create(&m_tid, NULL, callback, (void *)this);
}

void* ThreadMd5::callback(void* lpvoid)
{
	ThreadMd5* p = (ThreadMd5*)lpvoid;
	p->run();
	return p;
}

int ThreadMd5::run()
{
	int iPos = 0; 
	int ret  = 0;
	char szFile[300] = {0};
	char szTmpFile[300] = {0};

	for(;1;)
	{
		iPos = g_WorkPos.Pop();
		if(iPos == -1)
		{
			//printf("sleep pos: %d\n", iPos);
			Sleep(1);
			continue;
		}

		//not exist, continue
		strcpy(szFile, g_QueFile[iPos].c_str());
		sprintf(szTmpFile, "%s.df", szFile);

		g_FilePos.Push_W(iPos);

		if(access(szTmpFile, 0))
		{
			LOG("file %s not exist\n", szTmpFile);
			//g_FilePos.Push_W(iPos);
			continue;
		}

		ret = buildMd5(szFile);
		if(ret)
		{
			LOG("Build MD5 ret[%d]\n", ret);
		}

		//printf("thread %lu\n", m_tid);

	}
	return 0;
}

int ThreadMd5::buildMd5(const char* szFile)
{
	char szTmpFile[300] = {0};
	sprintf(szTmpFile, "%s.df", szFile);

	MD5 md5;
	struct stat stbuf;
	stat(szTmpFile, &stbuf);
	string strmd5;
	uint64_t ddwFileSize = (uint64_t)(stbuf.st_size);

	if(ddwFileSize > MAX_FILE_SIZE)
	{
		FILE* fpr = fopen(szTmpFile, "rb");
		if(!fpr)
		{
			LOG("can not open %s\n", szTmpFile);
			return -2;
		}

		int iDataSize = fread(m_pBuf, 1, MAX_SEND_SIZE, fpr);
		
		string strFile = string(m_pBuf, iDataSize);
		strFile.append((const char*)&ddwFileSize, 8);
		md5.update(strFile);
		strmd5 = md5.toString();
		fclose(fpr);
	}
	else
	{
		ifstream in(szTmpFile, ios::binary);
		if(!in.is_open())
		{
			LOG("can not open %s\n", szTmpFile);
			return -2;
		}

		md5.update(in);
		strmd5 = md5.toString();
		in.close();
	}

	
	char szMd5[300];
	sprintf(szMd5, "%s.md5", szFile);
	FILE* fpw = fopen(szMd5, "wb");
	
	if(!fpw)
	{
		LOG("Can not open %s, errno:[%d]\n", szMd5, errno);
		return -2;
	}
	fprintf(fpw, "%s", strmd5.c_str());
	fclose(fpw);

	LOG("%s [%ld Bytes] md5:'%s'\n", szFile, ddwFileSize, strmd5.c_str());
	return 0;
}

