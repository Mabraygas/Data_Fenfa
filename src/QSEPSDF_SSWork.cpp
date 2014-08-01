/**
 * @file QSEPSDF_SSWork.cpp
 * @brief 分发服务器端接收发送处理
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-09-24
 */

#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "QSEPSDF_SSWork.h"
#include "DayLog.h"
#include "uks_str.h"


#define MAXBUFLEN 128
#define  LOG(format, args...) g_Srvlog.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

#include "SmsNotify.h"
#include "Global.h"

extern DayLog   g_Srvlog;

extern string   g_QueFile[MAX_FILE_NUM];
extern QueEvent g_FilePos;
extern QueEvent g_WorkPos;


void QSEPSDF_SSWork::initialize()
{
	m_pBuf    = new char[MAX_SEND_SIZE];
	m_TimeOut = 60;
	pthread_mutex_init(&m_MainCri, NULL);

}

QSEPSDF_SSWork::~QSEPSDF_SSWork()
{
	pthread_mutex_destroy(&m_MainCri);
}

void QSEPSDF_SSWork::work(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	//Operate Type
	uint32_t iOpr = *(uint32_t*)(psBuf);

	switch(iOpr)
	{
		case DF_START_LST :
		   CreateAList(recv);
		   break;
		case DF_END_LST :
		   CloseAList(recv);
		   break;
		case DF_OPEN_FILE:
		   OpenAFile(recv);
		   break;
		case DF_CLOSE_FILE:
		   CloseAFile(recv);
		   break;
		case DF_WRITE_FILE:
		   WriteToFile(recv);
		   break;
		case DF_DEL_FILE:
		   RemoveFile(recv);
		   break;
		case DF_CREATE_FILE:
		   CreateAFile(recv);
		   break;
		case DF_CHECK_FILE:
		   CheckAFile(recv);
		   break;
        case DF_RENAME_FILE:
		   RenameFile(recv);
		   break;
		case DF_SINGLE_FILE:
		   RecvSingleFile(recv);
		   break;
		case DF_CHECK_MD5:
		   CheckMD5(recv);
		   break;
		case DF_GET_FILE_MD5:
		   GetFileMD5(recv);
		   break;
		case DF_FILE_CHMOD:
		   FileChmod(recv);
		   break;
		case DF_KILL_PROCESS:
		   KillProcess(recv);
		   break;
		case DF_EXEC_PROCESS:
		   ExecProcess(recv);
		   break;
		case DF_RPC_SHELL:
		   RPCShell(recv);
		   break;

		default:
		   LOG("[%s] Opereate Type:[%d] error!\n", recv.ip.c_str(), iOpr);
#ifdef NLOG
		   //cout<<"Opereate Type: "<<iOpr<<"error!"<<endl;
#endif
		   break;

	}

}


/**
 * @brief 构造返回包
 *
 * @param [msg]  :  构造的返回包
 * @param [pData]:  传入需要构造返回包的信息
 */
void QSEPSDF_SSWork::BuildResponsePack(string& msg, _DFIELD_* pData)
{
	string result;
	result.append((char *)&pData->m_OperateType, 4);
	result.append((char *)&pData->m_NO, 4);
	result.append((char *)&pData->m_Result, 4);
	if(pData->m_Result < 0)
	{
		result.append((char *)&errno, 1);
	}

	Protocol head;
	head.body_len = result.size();

	Protocol::HeadToBuffer(head,msg);
	msg += result;
}

/**
 * @brief 处理超时数据
 * 数据在Adapter接收数据队列中的时间已经超过允许值.
 * 是否超时,是在WorkImp()中判断的.
 * 默认调用close(),直接关闭连接.
 *
 * @param [recv] : Adapter接收队列中的元素
 */
void QSEPSDF_SSWork::WorkTimeout(const RecvData &recv)
{
	time_t tmNow = time(NULL);
	if(tmNow - recv.timestamp > m_TimeOut)
	{
		//TODO send message and close handle
		char szMsg[300];
		snprintf(szMsg, sizeof(szMsg), "error: [%s:%d] Recv Time Out!", recv.ip.c_str(), recv.port);
		SendSmsNotify(szMsg);
		LOG("[%s:%d] Recv Time Out!\n", recv.ip.c_str(), recv.port);
		close(recv.uid);
	}
	
}


int QSEPSDF_SSWork::BuildMd5(const char* szFile, string& strmd5, char* pBuf)
{
	//fseek(fpr, 0, SEEK_END);
	//uint64_t ddwFileSize = ftell(fpr);

	char szTmpFile[300] = {0};
	sprintf(szTmpFile, "%s.df", szFile);
	MD5 md5;
	struct stat stbuf;
	stat(szTmpFile, &stbuf);
	uint64_t ddwFileSize = (uint64_t)(stbuf.st_size);

	if(ddwFileSize > MAX_FILE_SIZE)
	{
		FILE* fpr = fopen(szTmpFile, "rb");
		if(!fpr)
		{
			LOG("can not open %s\n", szTmpFile);
			return -2;
		}

		int iDataSize = fread(pBuf, 1, MAX_SEND_SIZE, fpr);
		
		string strFile = string(pBuf, iDataSize);
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

/**
 * @brief 打开文件，并存入List
 */

void QSEPSDF_SSWork::OpenAFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint64_t *)psBuf;
	psBuf += 8;

	tmpData.m_ListNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Listname, tmpData.m_ListNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_ListNameLen;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("Open File '%s'\n", tmpData.m_Filename);
	//Changed on 20130322, open tmp file
	char szTmpFile[300] = {0};
	
	tmpData.m_Result = 0;

	if(tmpData.m_Filename[0] == '\0')
	{
		tmpData.m_Result = -1;
		LOG("Recv File Name NULL\n");
	}
	else
	{
		string strFile = string(tmpData.m_Filename);
		char szMd5[300];
		sprintf(szMd5, "%s.md5", tmpData.m_Filename);
        remove(szMd5);
		//检查m_mapNameFile中是否已经有记录
		if(m_mapNameFile.find(strFile) != m_mapNameFile.end())
		{
			//tmpData.m_Result = 0;    //已经存在
			//close file
			fclose(m_mapNameFile[strFile]);
			m_mapNameFile.erase(strFile);
		}

		sprintf(szTmpFile, "%s.df", tmpData.m_Filename);
		//FILE* fpw = fopen(tmpData.m_Filename, "wb"); 
		FILE* fpw = fopen(szTmpFile, "wb"); 
		if(!fpw)
		{
			LOG("Open File error:'%s', errno:[%d]\n", szTmpFile, errno);
			tmpData.m_Result = -1;
		}
		else
		{

			//LOG(" Open File:'%s'\n",szTmpFile);
			m_mapNameFile[strFile] = fpw;
			m_strListName = string(tmpData.m_Listname);
			//对应列表
			if(m_mapList.find(m_strListName) != m_mapList.end())
			{
				//将文件名存入list
				fprintf(m_mapList[m_strListName], "%s\n", tmpData.m_Filename);
				fflush(m_mapList[m_strListName]);

			}else
			{
				//List not exist, reopen 
				string strTmpList = m_strListName + ".dftmp";
				FILE* fpwList = NULL; 

				if(access(strTmpList.c_str(), 0) == 0)
					fpwList = fopen(strTmpList.c_str(), "a");
				else
					fpwList = fopen(strTmpList.c_str(), "wb");

				if(!fpwList)
				{
					LOG("Open File error:'%s', errno:[%d]\n", m_strListName.c_str(), errno);
					tmpData.m_Result = -1;
				}
				else
				{
					m_mapList[m_strListName] = fpwList;
					fprintf(fpwList, "%s\n", tmpData.m_Filename);
					fflush(m_mapList[m_strListName]);

				}
			}
		}
	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] Open File: %s %d\n", recv.ip.c_str(), tmpData.m_Filename, getpid());
}

/**
 * @brief 创建指定的List,将文件名与文件指针存入m_mapList
 */

void QSEPSDF_SSWork::CreateAList(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint64_t *)(psBuf);
	psBuf += 8;
	tmpData.m_ListNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Listname, tmpData.m_ListNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_ListNameLen;

	//printf("Create List:%s\n", tmpData.m_Listname);
	
	tmpData.m_Result = 0;

	if(tmpData.m_Listname[0] == '\0')
	{
		tmpData.m_Result = -1;
		LOG("List File Name NULL\n");
	}
	else{
		//List File Name
		string strListName = string(tmpData.m_Listname);

		//Temp List Name,  with .dftmp
		//string strRealName = strListName.substr(0, strListName.rfind('.'));

		string strTmpName = strListName + ".dftmp";
		if(m_mapList.find(strListName) != m_mapList.end())
		{
			//已经打开list
			LOG("%s has open\n", tmpData.m_Listname);
			fclose(m_mapList[strListName]);
			m_mapList.erase(strListName);
		}

		FILE* fpw;
		//exist the List, then copy it to temp file
		if(rename(strListName.c_str(), strTmpName.c_str()) == 0)
		{
			LOG("%s exists\n", strListName.c_str());

			//open file append
			fpw = fopen(strTmpName.c_str(), "a"); 
			
		}else{
			//open file write
			fpw = fopen(strTmpName.c_str(), "wb"); 
		}

		if(!fpw)
		{
			LOG("Open File error:'%s', errno:[%d]\n", strTmpName.c_str(), errno);
			tmpData.m_Result = -1;

		}else
		{
			m_mapList[strListName] = fpw;
		}

	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] Create List: %s\n", recv.ip.c_str(), tmpData.m_Listname);

}

/**
 * @brief 数据写入文件
 */

void QSEPSDF_SSWork::WriteToFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	//char tmpBuf[1<<20];

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint64_t *)(psBuf);
	psBuf += 8;

	/*tmpData.m_ListNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Listname, tmpData.m_ListNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_ListNameLen;
	 */

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("get data file:%d\n", tmpData.m_FileNameLen);
	tmpData.m_DataLength  = *(uint32_t *)psBuf;
	psBuf += 4;
	/*tmpData.m_PData = tmpBuf;
	memcpy(tmpData.m_PData, psBuf, tmpData.m_DataSize);
	psBuf += tmpData.m_DataSize;
	 */

	tmpData.m_Result = 0;

	if(tmpData.m_Filename[0] == '\0')
	{
		tmpData.m_Result = -1;
		LOG("File Name NULL\n");
	}
	else
	{
		string strFile = string(tmpData.m_Filename);
		if(m_mapNameFile.find(strFile) != m_mapNameFile.end())
		{
			if (tmpData.m_DataLength != (int32_t)fwrite(psBuf, sizeof(char), tmpData.m_DataLength, m_mapNameFile[strFile]))
			{
				tmpData.m_Result = -2;
				LOG("data size error:[%d], err:[%d]\n", tmpData.m_DataLength, tmpData.m_Result);
			}
		}
		else
		{
			LOG("File '%s' Pointer not exists\n", tmpData.m_Filename);
		}
	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid, msg, recv.ip, recv.port);

}

/**
 * @brief 关闭文件，并删除m_mapNameFile对应的项
 */

void QSEPSDF_SSWork::CloseAFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint64_t *)psBuf;
	psBuf += 8;

	/*
	tmpData.m_ListNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Listname, tmpData.m_ListNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_ListNameLen;
	*/
	

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	tmpData.m_DataLength  = *(uint32_t *)psBuf;  //1: All , 0: Mod

	//printf("Close '%s'\n", tmpData.m_Filename);

    tmpData.m_Result = 0;
	char szTmpFile[300] = {0};

	string strFile = string(tmpData.m_Filename);

	if(tmpData.m_Filename[0] == '\0')
	{
		tmpData.m_Result = -1;
		LOG("File name NULL\n");
	}
	else
	{
		/*
		char szMd5[300];
		sprintf(szMd5, "%s.md5", tmpData.m_Filename);
		FILE* fpw = fopen(szMd5, "wb");
		if(!fpw)
		{
			LOG("Open %s error, errno:[%d]\n", szMd5, errno);
			tmpData.m_Result = -1;
		}
		else
		{
			fwrite(tmpData.m_Listname, 1, tmpData.m_ListNameLen, fpw);
			fclose(fpw);
		}
		*/
		sprintf(szTmpFile, "%s.df", tmpData.m_Filename);

		if(m_mapNameFile.find(strFile)!=m_mapNameFile.end())
		{
			// exist
			fflush(m_mapNameFile[strFile]);
			fclose(m_mapNameFile[strFile]);
			m_mapNameFile.erase(strFile);
		}
		else
		{
			// not exist
			LOG("File '%s' Pointer not exist\n", tmpData.m_Filename);
		}

	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	if(tmpData.m_Result == 0)
	{
		if(1 == tmpData.m_DataLength) //All, Build Md5
		{

			int iPos = 0;
			do{
				iPos = g_FilePos.Pop();
				if(iPos == -1)
				{
					//printf("Close sleep\n");
					Sleep(1);
				}

			}while(iPos == -1);

			g_QueFile[iPos] = strFile;
			//printf("File:%s,QueNO:%d\n", strFile.c_str(), iPos);

			g_WorkPos.Push_W(iPos);
			//string strTmp;
			//BuildMd5(tmpData.m_Filename, strTmp, m_pBuf);
		}
		else if(0 == tmpData.m_DataLength)
		{
			int ret = rename(szTmpFile, tmpData.m_Filename);
			if(ret)
			{
				LOG("Rename error:'%s' -> '%s'\n", szTmpFile, tmpData.m_Filename);
			}
		}
	}

	SendResponse(recv.uid,msg,recv.ip,recv.port);

	LOG("[%s] Close File: %s %d\n", recv.ip.c_str(), tmpData.m_Filename, getpid());
		
}

/**
 * @brief 关闭List文件，删除m_mapList对应的项
 */

void QSEPSDF_SSWork::CloseAList(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();
	_DFIELD_ tmpData;

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint64_t *)psBuf;
	psBuf += 8;
	tmpData.m_ListNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Listname, tmpData.m_ListNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_ListNameLen;
	tmpData.m_DatePrint   = *(uint32_t*)psBuf;
	psBuf += 4;
	//printf("Close List '%s'\n", tmpData.m_Listname);

	/********************************
	  获取系统当前日期
	  ******************************/
	time_t nowtime;
	time(&nowtime);
	struct tm* ConvertedTime;
	ConvertedTime = gmtime(&nowtime);
	char TimeString[300];
	//snprintf(TimeString, sizeof(TimeString), ConvertedTime->tm_mon > 8 ? ".%d%d%d" : ".%d0%d%d", 1900 + ConvertedTime->tm_year, 1 + ConvertedTime->tm_mon, ConvertedTime->tm_mday);
	snprintf(TimeString, sizeof(TimeString), 
             (ConvertedTime->tm_mon > 8 && ConvertedTime->tm_mday > 8) ?  ".%d%d%d" : 
             (ConvertedTime->tm_mon > 8 || ConvertedTime->tm_mday > 8) ? (ConvertedTime->tm_mon > 8 ? ".%d%d0%d" : ".%d0%d%d") :
             ".%d0%d0%d", 1900 + ConvertedTime->tm_year, 1 + ConvertedTime->tm_mon, ConvertedTime->tm_mday);
    tmpData.m_Result = 0;
	string strListName = string(tmpData.m_Listname);
	//if(tmpData.m_DatePrint == 1)
	//	strListName += string(TimeString);
	//cout<<tmpData.m_DatePrint<<" "<<strListName<<endl;	
	if(m_mapList.find(strListName) != m_mapList.end())
	{
		fclose(m_mapList[strListName]);
		m_mapList.erase(strListName);

		string strTmpList = strListName + ".dftmp";
		if(tmpData.m_DatePrint == 1)
			strListName += string(TimeString);
		if(rename(strTmpList.c_str(), strListName.c_str()))
		{
			LOG("rename: %s -> %s, errno:[%d]\n", strTmpList.c_str(), strListName.c_str(), errno);
			tmpData.m_Result = -1;
		}
		LOG("rename: %s -> %s\n", strTmpList.c_str(), strListName.c_str());
		//create send end symbol
		/*
		char szSymbol[300];
		sprintf(szSymbol, "%s.RecvEnd", tmpData.m_Listname);

		FILE* fpw = fopen(szSymbol, "wb");
		if(!fpw)
		{
			LOG("Create '%s' error, errno:[%d]\n", szSymbol, errno);
			tmpData.m_Result = -1;
		}
		else
		{
			fclose(fpw);
		}*/

	}
	else{
		tmpData.m_Result = -2;
		LOG("List %s not exist, cannot close\n", tmpData.m_Listname);
	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid,msg,recv.ip,recv.port);
	LOG("[%s] Close List: %s\n", recv.ip.c_str(), tmpData.m_Listname);

}

/**
 * @brief 删除指定的文件
 */
void QSEPSDF_SSWork::RemoveFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;
	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("Remove '%s'\n", tmpData.m_Filename);

	tmpData.m_Result = 0;
	if(remove(tmpData.m_Filename))
	{
		LOG("Remove '%s' error, errno:[%d]\n", tmpData.m_Filename, errno);
		tmpData.m_Result = -1;
	}
	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid,msg,recv.ip,recv.port);
	LOG("[%s] Remove File: %s\n" , recv.ip.c_str(), tmpData.m_Filename);

}

/**
 * @brief 创建文件
 *
 */
void QSEPSDF_SSWork::CreateAFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("Create '%s'\n", tmpData.m_Filename);

	//含有少量数据
	if(recv.buffer.size() > (size_t)(12 + tmpData.m_FileNameLen))
	{
		tmpData.m_DataLength = *(uint32_t *)psBuf;
		psBuf += 4;
	}

	tmpData.m_Result = 0;
	if(access(tmpData.m_Filename, 0) == 0)
	{
		LOG("File already exists\n");
		if(tmpData.m_DataLength > 0)
		{
			FILE* fpw = fopen(tmpData.m_Filename, "a");
			if(!fpw)
			{
				LOG("Create '%s' error, errno:[%d]\n", tmpData.m_Filename, errno);
				tmpData.m_Result = -1;
			}
			else
			{
				tmpData.m_Result = fwrite(psBuf, 1, tmpData.m_DataLength, fpw);
				fclose(fpw);
			}
		}
	}
	else
	{
		FILE* fpw = fopen(tmpData.m_Filename, "wb");
		if(!fpw)
		{
			LOG("Create '%s' error, errno:[%d]\n", tmpData.m_Filename, errno);
			tmpData.m_Result = -1;
		}
		else
		{
			if(tmpData.m_DataLength > 0)
			{
				tmpData.m_Result = fwrite(psBuf, 1, tmpData.m_DataLength, fpw);
			}
			fclose(fpw);
		}
	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] Create File: %s\n" , recv.ip.c_str(), tmpData.m_Filename);

}

/**
 * @brief 检测文件存在
 *
 */
void QSEPSDF_SSWork::CheckAFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("Check '%s'\n", tmpData.m_Filename);

	tmpData.m_Result = 0;
	if(access(tmpData.m_Filename, 0))
	{
		LOG("File '%s' not exists, errno:[%d]\n", tmpData.m_Filename, errno);
		tmpData.m_Result = -1;
	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] Check File: %s\n" , recv.ip.c_str(), tmpData.m_Filename);

}

/**
 * @brief rename file
 *
 */
void QSEPSDF_SSWork::RenameFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

    tmpData.m_ListNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Listname, tmpData.m_ListNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_ListNameLen;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("rename %s -> %s\n", tmpData.m_Listname, tmpData.m_Filename);

	tmpData.m_Result = 0;
	if(rename(tmpData.m_Listname, tmpData.m_Filename))
	{
		LOG("rename %s -> %s, errno:[%d]\n", tmpData.m_Listname, tmpData.m_Filename, errno);
		tmpData.m_Result = -1;
	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] Rename %s -> %s\n" , recv.ip.c_str(), tmpData.m_Listname, tmpData.m_Filename);

}

/**
 * @brief 接收单个文件
 *
 */
void QSEPSDF_SSWork::RecvSingleFile(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("Single '%s'\n", tmpData.m_Filename);

	tmpData.m_Result = 0;
	char szTmpFile[300] = {0};

	if(tmpData.m_Filename[0] == '\0')
	{
		tmpData.m_Result = -1;
		LOG("Recv File Name NULL\n");
	}
	else
	{
		string strFile = string(tmpData.m_Filename);
		//检查m_mapNameFile中是否已经有记录
		if(m_mapNameFile.find(strFile) != m_mapNameFile.end())
		{
			//tmpData.m_Result = 0;    //已经存在
			fclose(m_mapNameFile[strFile]);
			m_mapNameFile.erase(strFile);

		}
		sprintf(szTmpFile, "%s.df", tmpData.m_Filename);
		//文件还没打开
		FILE* fpw = fopen(szTmpFile, "wb"); 
		if(!fpw)
		{
			LOG("Open File error:'%s', errno:[%d]\n", szTmpFile, errno);
			tmpData.m_Result = -2;
		}else
		{
			m_mapNameFile[strFile] = fpw;
		}

	}

	string msg;
	BuildResponsePack(msg, &tmpData);

	SendResponse(recv.uid,msg,recv.ip,recv.port);
	LOG("[%s] Recv Single File: %s\n", recv.ip.c_str(), tmpData.m_Filename);

}

/**
 * @brief MD5校验
 *
 */
void QSEPSDF_SSWork::CheckMD5(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	char szMD5[33];
	char szMD5File[200];

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	int iMd5Len           = *(uint32_t *)psBuf;
	psBuf += 4;
	memcpy(szMD5, psBuf, iMd5Len);
	szMD5[32] = '\0';

	sprintf(szMD5File, "%s.md5", tmpData.m_Filename);

	//printf("recv:%s\n", szMD5);

	tmpData.m_Result = 0;

	char szTmpFile[300] = {0};
	sprintf(szTmpFile, "%s.df", tmpData.m_Filename);

	struct stat buf;
	stat(szTmpFile, &buf);

	if(buf.st_mode & S_IFDIR)
	{
		//dir
		tmpData.m_Result = -1;
		LOG("%s is Dir\n", szTmpFile);
	}
	else if(access(szTmpFile, 0))
	{
		tmpData.m_Result = 0;
		LOG("%s not exist\n", szTmpFile);
	}
	else
	{
		if(access(szMD5File, 0) == 0)
		{
			//exist md5 file
			FILE* fpr = fopen(szMD5File, "rb");
			if(fpr!=NULL)
			{
				char szTmpMD5[33];
				if(fgets(szTmpMD5, 33, fpr) != NULL)
				{
					if(strcmp(szMD5, szTmpMD5) != 0)
					{
						tmpData.m_Result = -2;
						LOG("check file '%s' fail:client md5:'%s',\r\nserver md5:'%s'\n", szMD5File, szMD5, szTmpMD5);

					}
					else
					{
						LOG("check '%s',md5:'%s'\n", tmpData.m_Filename, szTmpMD5);
						int ret = rename(szTmpFile, tmpData.m_Filename);
						if(ret)
						{
							LOG("Rename error:'%s'->'%s'\n", szTmpFile, tmpData.m_Filename);
						}

						ret = remove(szMD5File);
						if(ret)
						{
							LOG("Remove error:'%s'\n", szMD5File);
						}

					}
				}
				fclose(fpr);
				fpr = NULL;
			}
			else
			{
				tmpData.m_Result = -3;
				LOG("Open %s fail!\n", szMD5File);
			}
		}
		else
		{
			string strMd5;
			int ret = BuildMd5(tmpData.m_Filename, strMd5, m_pBuf);
			if(ret < 0)
			{
				tmpData.m_Result = -4;
				LOG("%s Build Md5 fail,ret:[%d]\n", tmpData.m_Filename, ret);
			}
			else
			{
				LOG("File:%s, md5:%s\n", tmpData.m_Filename, strMd5.c_str());
				if(strcmp(szMD5, strMd5.c_str()) != 0)
				{
					tmpData.m_Result = -2;
					LOG("check '%s' fail:client md5:'%s',\r\nserver md5:'%s'\n", tmpData.m_Filename, szMD5, strMd5.c_str());
				}
				else
				{
					ret = rename(szTmpFile, tmpData.m_Filename);
					if(ret)
					{
						LOG("Rename error:'%s'->'%s'\n", szTmpFile, tmpData.m_Filename);
					}

					ret = remove(szMD5File);
					if(ret)
					{
						LOG("Remove error:'%s'\n", szMD5File);
					}

				}
			}

		}

	}
		
	/*
	if(tmpData.m_Result < 0)
	{
		LOG("File %s MD5 check failue!\n", tmpData.m_Filename);
	}
	*/

	string result;
	string msg;
	result.append((char *)&tmpData.m_OperateType, 4);
	result.append((char *)&tmpData.m_NO, 4);
	result.append((char *)&tmpData.m_Result, 4);


	Protocol head;
	head.body_len = result.size();

	Protocol::HeadToBuffer(head,msg);
	msg += result;

	SendResponse(recv.uid, msg, recv.ip, recv.port);

	LOG("[client ip:%s]\n", recv.ip.c_str());
}

/**
 * @brief MD5校验
 *
 */
void QSEPSDF_SSWork::GetFileMD5(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	char szMD5[33];
	char szMD5File[200];

	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	sprintf(szMD5File, "%s.md5", tmpData.m_Filename);

	tmpData.m_Result = 0;

	if(access(tmpData.m_Filename, 0))
	{
		//not exist
		tmpData.m_Result = -1;
	}
	else
	{
		if(access(szMD5File, 0) == 0)
		{
			//exist md5 file
			FILE* fpr = fopen(szMD5File, "rb");
			if(fpr!=NULL)
			{
				if(fgets(szMD5, 33, fpr) == NULL)
				{
					tmpData.m_Result = -2;
				}
				fclose(fpr);
				fpr = NULL;
			}
			else
			{
				tmpData.m_Result = -3;
			}
		}
		else
		{
			ifstream in(tmpData.m_Filename, ios::binary);
			if(!in.is_open())
			{
				tmpData.m_Result = -3;
			}
			else
			{
				MD5 md5(in);
				LOG("File:%s, md5:%s\n", tmpData.m_Filename, md5.toString().c_str());
				in.close();

				strcpy(szMD5, md5.toString().c_str());
			}
		}

	}

	
	string result;
	string msg;
	result.append((char *)&tmpData.m_OperateType, 4);
	result.append((char *)&tmpData.m_NO, 4);
	result.append((char *)&tmpData.m_Result, 4);
	if(tmpData.m_Result == 0)
	{
		result.append(szMD5, 32);
	}


	Protocol head;
	head.body_len = result.size();

	Protocol::HeadToBuffer(head,msg);
	msg += result;

	SendResponse(recv.uid, msg, recv.ip, recv.port);

	LOG("[client ip:%s]\n", recv.ip.c_str());
}

/**
 * @brief 给文件增加执行权限
 *
 */
void QSEPSDF_SSWork::FileChmod(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	//printf("chmod File '%s'\n", tmpData.m_Filename);

	tmpData.m_Result = 0;

	struct stat buf;
	stat(tmpData.m_Filename, &buf);

	if(S_ISDIR(buf.st_mode))
	{
		//dir
		tmpData.m_Result = -1;
		LOG("%s is Dir\n", tmpData.m_Filename);

	}
	else if (tmpData.m_Filename[0] == '/' && access(tmpData.m_Filename, 0))
	{
		tmpData.m_Result = -2;
		LOG("%s not exist\n", tmpData.m_Filename);

	}
	else if(tmpData.m_OperateType != DF_FILE_CHMOD)
	{
		LOG("Operate Type error [%d]\n", tmpData.m_OperateType);
		tmpData.m_Result = -255;
	}
	else if(access(tmpData.m_Filename, 0))
	{
		LOG("File '%s' not exists, errno:[%d]\n", tmpData.m_Filename, errno);
		tmpData.m_Result = -1;
	}
	else
	{
		if(chmod(tmpData.m_Filename, S_IXUSR))
		{
			LOG("chmod '%s' error, errno:[%d]\n", tmpData.m_Filename, errno);
			tmpData.m_Result = -2;
		}
	}

	string result;
	string msg;
	result.append((char *)&tmpData.m_OperateType, 4);
	result.append((char *)&tmpData.m_NO, 4);
	result.append((char *)&tmpData.m_Result, 4);


	Protocol head;
	head.body_len = result.size();

	Protocol::HeadToBuffer(head,msg);
	msg += result;

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] Chmod File: %s\n" , recv.ip.c_str(), tmpData.m_Filename);

}

/**
 * @brief 杀死进程
 *
 */
void QSEPSDF_SSWork::KillProcess(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	tmpData.m_Result = 0;
	string strFile = tmpData.m_Filename;

	pthread_mutex_lock(&m_MainCri);

	multimap<string, pid_t>::const_iterator cit = m_mapProcPid.find(strFile);

	if(tmpData.m_OperateType != DF_KILL_PROCESS)
	{
		LOG("Operate Type error [%d]\n", tmpData.m_OperateType);
		tmpData.m_Result = -255;
	}
	/*
	else if(access(tmpData.m_Filename, 0))
	{
		LOG("File '%s' not exists, errno:[%d]\n", tmpData.m_Filename, errno);
		tmpData.m_Result = -1;
	}
	*/
	else if(cit != m_mapProcPid.end())
	{
		pid_t PID;
		int status = 0;

		while(cit != m_mapProcPid.end())
		{

			PID = cit->second;
			cit++;

			if(kill(PID, SIGKILL) < 0)
			{
				LOG("can not kill %s -> %d, errno:[%d]\n", tmpData.m_Filename, PID, errno);
				if(errno == ESRCH)
				{
					LOG("Process %s not exist\n", tmpData.m_Filename);
					tmpData.m_Result = 0;
				}
				else
				{
					tmpData.m_Result = -2;
				}
			}

			while(waitpid(PID, &status, 0) < 0)
			{

				if (errno != EINTR)
				{
					tmpData.m_Result = 0;
					break;
				}
				Sleep(1);
			}

			LOG("Kill Process %s pid:%d\n", tmpData.m_Filename, PID);

		}

		
		m_mapProcPid.erase(strFile);

	}
	else
	{
		char szShell[300];
		char szFile[300];
		if(tmpData.m_Filename[0] == '/')
		{
			char *p = strrchr(tmpData.m_Filename, '/');
			strcpy(szFile, p + 1);
		}
		else
		{
			strcpy(szFile, tmpData.m_Filename);
		}
		sprintf(szShell, "ps -ef | grep %s | grep -v grep | grep -v xtool_dfscp | awk '{print $2}'", szFile);
		vector<int> vec;
		FILE* pfr = popen(szShell, "r");
		char szBuf[20];
		while(fgets(szBuf, 20, pfr) != NULL)
		{
			LocalRTrim(szBuf);
			vec.push_back(atoi(szBuf));
		}
		pclose(pfr);

		pid_t PID;
		int status;
		int iLoop = 0;
		int iret = 0;
		for(vector<int>::iterator itr = vec.begin(); itr != vec.end(); itr++)
		{
			PID=*itr;
			if(PID <= 0)
				continue;
			if(kill(PID, SIGKILL) < 0)
			{
				LOG("can not kill %s -> %d, errno:[%d]\n", tmpData.m_Filename, PID, errno);
				if(errno == ESRCH)
				{
					LOG("Process %s not exist\n", tmpData.m_Filename);
					tmpData.m_Result = 0;
				}
				else
				{
					tmpData.m_Result = -2;
				}
				break;
			}

			while(waitpid(PID, &status, 0) < 0)
			{

				if(iLoop ++ > 2)
				{
					iret -= iLoop;
					iLoop = 0;
					break;
				}

				if (errno != EINTR)
				{
					tmpData.m_Result = 0;
					iret = 0;
					break;
				}
				Sleep(1);
			}

			if(iret < 0)
			{
				tmpData.m_Result = iret;
				break;
			}
		}

		/*
		char szProc[300];
		sprintf(szProc, "/var/run/dfs.pid");

		PID = 0;
		FILE* fpr = fopen(szProc, "rb");
		if(fpr == NULL)
		{
			tmpData.m_Result = -1;
			LOG("open %s error!\n", szProc);
		}
		else
		{
			char szPID[20];
			if(fgets(szPID, 20, fpr) != NULL)
			{
				LocalRTrim(szPID);
				PID = atoi(szPID);
			}

			fclose(fpr);
			remove(szProc);
		}

		if(PID > 0)
		{
			if(kill(PID, SIGKILL) < 0)
			{
				LOG("can not kill %s -> %d, errno:[%d]\n", tmpData.m_Filename, PID, errno);
				if(errno == ESRCH)
				{
					LOG("Process %s not exist\n", tmpData.m_Filename);
					tmpData.m_Result = 0;
				}
				else
				{
					tmpData.m_Result = -2;
				}
			}

			int status;
			while(waitpid(PID, &status, 0) < 0)
			{

				if (errno != EINTR)
				{
					tmpData.m_Result = 0;
					break;
				}
				Sleep(1);
			}

		}
		*/
		
		
	}

	pthread_mutex_unlock(&m_MainCri);
	if(0)
	{
		char szProc[300];
		sprintf(szProc, "%s.PID", tmpData.m_Filename);
		FILE* fpr = fopen(szProc, "rb");
		if(fpr == NULL)
		{
			tmpData.m_Result = -1;
			LOG("open %s error!\n", szProc);
		}
		else
		{
			char szPID[20];
			if(fgets(szPID, 20, fpr) != NULL)
			{
				LocalRTrim(szPID);
				pid_t PID = atoi(szPID);

				if(kill(PID, SIGKILL) < 0)
				{
					LOG("can not kill %s -> %d, errno:[%d]\n", tmpData.m_Filename, PID, errno);
					if(errno == ESRCH)
					{
						LOG("Process %s not exist\n", tmpData.m_Filename);
						tmpData.m_Result = 0;
					}
					else
					{
						tmpData.m_Result = -2;
					}
				}

				int status;
				while(waitpid(PID, &status, 0) < 0)
				{

					if (errno != EINTR)
					{
						tmpData.m_Result = 0;
						break;
					}
					Sleep(1);
				}


			}

			fclose(fpr);
			remove(szPID);
        }
	}

	string result;
	string msg;
	result.append((char *)&tmpData.m_OperateType, 4);
	result.append((char *)&tmpData.m_NO, 4);
	result.append((char *)&tmpData.m_Result, 4);


	Protocol head;
	head.body_len = result.size();

	Protocol::HeadToBuffer(head,msg);
	msg += result;

	SendResponse(recv.uid, msg, recv.ip, recv.port);
	LOG("[%s] kill process %s End\n" , recv.ip.c_str(), tmpData.m_Filename);

}

/**
 * @brief 启动进程
 *
 */
void QSEPSDF_SSWork::ExecProcess(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	tmpData.m_Result = 0;

	char* argv[20] = {0};
	char* p = tmpData.m_Filename;
	char* pre = p;
	int iCnt = 0;

	while(*p && iCnt < 19)
	{
		while(*p && *p != ' ')
			p++;
		argv[iCnt] = pre;
		if(*p == 0)
			break;

		iCnt++;
		*p = 0;
		p++;
		pre = p;
	}

	struct stat buf;
	stat(argv[0], &buf);

	if(S_ISDIR(buf.st_mode))
	{
		//dir
		tmpData.m_Result = -1;
		LOG("%s is Dir\n", argv[0]);

	}
	else if (argv[0][0] == '/' && access(argv[0], 0))
	{
		tmpData.m_Result = -2;
		LOG("%s not exist\n", argv[0]);

	}
	else if(tmpData.m_OperateType != DF_EXEC_PROCESS)
	{
		LOG("Operate Type error [%d]\n", tmpData.m_OperateType);
		tmpData.m_Result = -255;

	}
	else
	{
		pid_t PID = fork();
		if(PID == 0)
		{
			//防止子进程继承父进程的socket
			fcntl(work_group_->adapter->getSocket().getfd(), F_SETFD, FD_CLOEXEC);
			if(execvp(argv[0], argv) < 0)
			{
				tmpData.m_Result = -1;
				LOG("execute %s errno:[%d]\n", argv[0], errno);
				exit(-1);
			}
			LOG("execute %s exit\n", argv[0]);
			exit(-1);
		}
		else
		{
			if(PID > 0)
			{
				
				pthread_mutex_lock(&m_MainCri);

				m_mapProcPid.insert(make_pair(string(argv[0]), PID));

				pthread_mutex_unlock(&m_MainCri);

				char szProcess[300];
				sprintf(szProcess, "/var/run/dfs.pid");
				FILE* fpw = fopen(szProcess, "ab");
				if(fpw == NULL)
				{
					tmpData.m_Result = -1;
				}
				else
				{
					fprintf(fpw, "%s\t%d\n", argv[0], PID);
				}
				fclose(fpw);

				char szShell[300];
				sprintf(szShell, "ps -ef | grep %d | grep -v grep | awk '{print $2}'", PID);
				FILE* pfr = popen(szShell, "r");
				char szBuf[20];
				if(fgets(szBuf, 20, pfr) == NULL)
				{
					tmpData.m_Result = -4;
				}
				pclose(pfr);

			}
			else
			{
				tmpData.m_Result = -3;
			}

			string result;
			string msg;
			result.append((char *)&tmpData.m_OperateType, 4);
			result.append((char *)&tmpData.m_NO, 4);
			result.append((char *)&tmpData.m_Result, 4);


			Protocol head;
			head.body_len = result.size();

			Protocol::HeadToBuffer(head,msg);
			msg += result;

			SendResponse(recv.uid, msg, recv.ip, recv.port);
			LOG("[%s] Exec %s, pid:%d\n" , recv.ip.c_str(), argv[0], PID);
			tmpData.m_Result = 0;

		}
	}
	
	if(tmpData.m_Result)
	{
		string result;
		string msg;
		result.append((char *)&tmpData.m_OperateType, 4);
		result.append((char *)&tmpData.m_NO, 4);
		result.append((char *)&tmpData.m_Result, 4);

		Protocol head;
		head.body_len = result.size();

		Protocol::HeadToBuffer(head,msg);
		msg += result;

		SendResponse(recv.uid, msg, recv.ip, recv.port);

		LOG("[%s] Exec %s\n" , recv.ip.c_str(), argv[0]);
	}

}

/**
 * @brief RPC
 *
 */
void QSEPSDF_SSWork::RPCShell(const RecvData &recv)
{
	const char *psBuf = (recv.buffer).c_str();

	_DFIELD_ tmpData;
	tmpData.m_OperateType = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_NO          = *(uint32_t *)psBuf;
	psBuf += 4;
	tmpData.m_OPNO        = *(uint32_t *)psBuf;
	psBuf += 8;

	tmpData.m_FileNameLen = *(uint32_t *)psBuf;
	psBuf += 4;
	snprintf(tmpData.m_Filename, tmpData.m_FileNameLen + 1, "%s", psBuf);
	psBuf += tmpData.m_FileNameLen;

	tmpData.m_Result = 0;

	string strres;

	if(tmpData.m_OperateType != DF_RPC_SHELL)
	{
		LOG("Operate Type error [%d]\n", tmpData.m_OperateType);
		tmpData.m_Result = -255;
	}
	else
	{
		FILE* pfr = popen(tmpData.m_Filename, "r");
		if(pfr == NULL)
		{
			strres="rpc error"+string(tmpData.m_Filename);
			tmpData.m_Result = -4;
			LOG("%s error [%d]\n", tmpData.m_Filename, errno);
		}
		else
		{
			char szBuf[2048] = {0};
			fread(szBuf, 1, 2048, pfr);
			strres = szBuf;
		}
		pclose(pfr);

	}

	string result;
	string msg;
	int size=0;
	size = strres.size();
	result.append((char *)&tmpData.m_OperateType, 4);
	result.append((char *)&tmpData.m_NO, 4);
	result.append((char *)&tmpData.m_Result, 4);

	result.append((char *)&size, 4);
	result+=strres;


	Protocol head;
	head.body_len = result.size();

	Protocol::HeadToBuffer(head,msg);
	msg += result;

	SendResponse(recv.uid, msg, recv.ip, recv.port);

	LOG("[%s] RPC:%s \n", recv.ip.c_str(), tmpData.m_Filename);
}

