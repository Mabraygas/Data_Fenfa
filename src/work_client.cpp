/**
 * @file workclient.cpp
 * @brief 通用客户端
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-09
 */

#include <iostream>

#include "QSEPSDF_CS.h"
#include "md5.h"
#include "protocol.h"
#include "DayLog.h"

#include "work_client.h"
#define  MAX_RECV_BUF  1024
#define  MAX_SEND_BUF  1048576   //1M
//#define  MAX_FILE_SIZE 62914560  //60M

#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

extern DayLog g_log;

using namespace std;

WorkClient::WorkClient()
{
	m_iServerPort = 51116;
	m_tcpclient   = NULL;
}

WorkClient::WorkClient(const string& strIP, const uint32_t iPort)
{
	m_strServerIP = strIP;
	m_iServerPort = iPort;
	m_tcpclient = new TCPClient(m_strServerIP, m_iServerPort);
}

WorkClient::~WorkClient()
{
	if(m_tcpclient)
	{
		delete m_tcpclient;
		close();
	}
}

/*void WorkClient::setIP(const string& strIP)
{
	m_strServerIP = strIP;
}

void WorkClient::setPort(const uint32_t iPort)
{
	m_iServerPort = iPort;
}
*/

int WorkClient::Initialize()
{
	if(m_tcpclient == NULL)
		m_tcpclient = new TCPClient(m_strServerIP, m_iServerPort, 10000);
	return 0;
}

void WorkClient::close()
{
	m_tcpclient->close();
}

int WorkClient::sendRequest(const string& body, string& result)
{
	if(body.size() <=0 )
	{
		return -1;
	}

	Protocol head;	
	head.body_len = body.size();	

	string msg;	
	Protocol::HeadToBuffer(head, msg);	
	msg += body;

	//send package
	string err;
	int ret = m_tcpclient->send(msg, err);
	if(ret < 0){             
		LOG(" send failed:%s\n", err.c_str()) ;     
		//assert(false);        
		return ret;
	}   


	//receive data
	char recvBuf[MAX_RECV_BUF];
	size_t len = MAX_RECV_BUF;

	ret = m_tcpclient->recv(recvBuf, len, err);
	if(ret < 0){             
		LOG(" recv failed:%s\n", err.c_str());            
		printf(" recv failed:%s\n", err.c_str());            
		//assert(false);        
		return ret;
	} 

	//parsing recv data
	string strBuf = string(recvBuf, len);
	if(Protocol::ParseProtocol(strBuf, result) != PACKET_FULL)
	{
		LOG("Package error\n");
		return -2;
	}

	if(result.length() < 12)
	{
		LOG("Package body length error[%d]\n", (int)result.length());
		return -3;
	}
	return 0;

}

int WorkClient::parseResult(string& result, _DFIELD_* pData)
{
	const char *psBuf = result.c_str();
	int32_t Opt = *(int32_t *)psBuf;
	psBuf += 4;
	if(Opt != pData->m_OperateType)
	{
		LOG("Opt error,Send [%d]--Recv [%d]\n", Opt, pData->m_OperateType);
		return -3;
	}

	int32_t NO = *(int32_t *)psBuf;
	psBuf += 4;
	if(NO != pData->m_NO)
	{
		LOG("NO error,Send [%d]--Recv [%d]\n", NO, pData->m_NO);
		return -4;
	}

	pData->m_Result = *(uint32_t *)psBuf;
	psBuf += 4;

	if(pData->m_Result < 0)
	{
		LOG("error!,ret:[%d]\n", pData->m_Result);
		return pData->m_Result;
	}
	return 0;
}

int WorkClient::send(_DFIELD_* pData)
{
	int ret = 0;
	string body, result;

	ret = buildDataToStr(pData, body);
	if(ret)
	{
		LOG("Build String error!, ret:[%d]\n", ret);
		return ret;
	}

	ret = sendRequest(body, result);
	if(ret)
	{
		LOG("Send Data error, ret:[%d]\n", ret);
		return ret;
	}

	ret = parseResult(result, pData);
	if(ret)
	{
		LOG("Parse ret:[%d]\n", ret);
		return ret;
	}

	return 0;
}

int WorkClient::commonWork(const char* file, int iOper)
{
	assert(file != NULL);

	if(iOper < 0 || iOper > 255)
	{
		return -255;
	}

	_DFIELD_ tmpData;

	strcpy(tmpData.m_Filename, file);
	tmpData.m_OperateType = iOper;
	tmpData.m_FileNameLen = strlen(tmpData.m_Filename);
	tmpData.m_NO          = tmpData.m_FileNameLen;
	tmpData.m_ListNameLen = 0;
	tmpData.m_DataLength  = 0;
	tmpData.m_OPNO        = 0;

	return send(&tmpData);
}

int WorkClient::buildMd5(const char* szFile, string& strmd5, char* pBuf)
{
	FILE* fpr = fopen(szFile, "rb");
	if(!fpr)
	{
		LOG("can not open %s\n", szFile);
		return -2;
	}
	fseek(fpr, 0, SEEK_END);
	uint64_t ddwFileSize = ftell(fpr);
	MD5 md5;

	if(ddwFileSize > MAX_FILE_SIZE)
	{
		rewind(fpr);
		int iDataSize = fread(pBuf, 1, MAX_SEND_BUF, fpr);
		
		string strFile = string(pBuf, iDataSize);
		strFile.append((const char*)&ddwFileSize, 8);
		md5.update(strFile);
		strmd5 = md5.toString();
		fclose(fpr);
	}
	else
	{
		fclose(fpr);
		ifstream in(szFile, ios::binary);
		if(!in.is_open())
		{
			LOG("can not open %s\n", szFile);
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

	return 0;
}
/*
int WorkClient::buildMd5(const char* szFile, string& strmd5)
{
	ifstream in(szFile, ios::binary);
	if(!in.is_open())
	{
		printf("can not open %s\n", szFile);
		return -2;
	}

	MD5 md5(in);

	strmd5 = md5.toString();
	in.close();

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

	return 0;
}
*/

int WorkClient::checkMD5(const char* psLocalFile, const char* psRemoteFile)
{
	if(psLocalFile == NULL || psRemoteFile == NULL)
	{
		return -1;
	}

	int iLen = strlen(psRemoteFile);
	if('/' == psRemoteFile[iLen - 1])
		return -2;

	string strMd5;

	char szMd5[300];
	sprintf(szMd5, "%s.md5", psLocalFile);
	FILE* fprMd5 = fopen(szMd5, "rb");
	if(!fprMd5)
	{
		char* pBuf = new char[MAX_SEND_SIZE];
		buildMd5(psLocalFile, strMd5, pBuf);
		delete [] pBuf;
	}
	else
	{
		char szTmp[33];
		fread(szTmp, 1, 32, fprMd5);
		szTmp[32] =  '\0';
		strMd5 = string(szTmp);
		fclose(fprMd5);
		remove(szMd5);
	}

	int iMd5Len = (int)strMd5.size();


	_DFIELD_ tmpData;
	int ret = 0;

	strcpy(tmpData.m_Filename, psRemoteFile);
	tmpData.m_OperateType = DF_CHECK_MD5;
	tmpData.m_FileNameLen = strlen(tmpData.m_Filename);
	tmpData.m_NO          = tmpData.m_FileNameLen;
	tmpData.m_OPNO        = 0;

	string result;
	string body;

	body.append((const char*)&tmpData.m_OperateType, 4);
	body.append((const char*)&tmpData.m_NO, 4);
	body.append((const char*)&tmpData.m_OPNO, 8);

	body.append((const char*)&tmpData.m_FileNameLen, 4);
	body.append(tmpData.m_Filename, tmpData.m_FileNameLen);

	body.append((const char*)&iMd5Len, 4);
	body.append(strMd5.c_str(), iMd5Len);

	do{
		ret = sendRequest(body, result);
		if(ret < 0)
		{
			LOG("Send Data error, ret:[%d]\n", ret);
			//return ret;
			usleep(100);
		}
	}while(ret < 0);

	ret = parseResult(result, &tmpData);
	if(ret)
	{
		LOG("parse ret:[%d]\n", ret);
		return ret;
	}

	return 0;
}

int WorkClient::getMD5(const char* psRemoteFile, string& strmd5)
{
	if(psRemoteFile == NULL)
	{
		return -1;
	}

	_DFIELD_ tmpData;

	strcpy(tmpData.m_Filename, psRemoteFile);
	tmpData.m_OperateType = DF_GET_FILE_MD5;
	tmpData.m_FileNameLen  = strlen(tmpData.m_Filename);
	tmpData.m_NO          = tmpData.m_FileNameLen;

	string result;
	string body;

	body.append((const char*)&tmpData.m_OperateType, 4);
	body.append((const char*)&tmpData.m_NO, 4);
	body.append((const char*)&tmpData.m_OPNO, 8);

	body.append((const char*)&tmpData.m_FileNameLen, 4);
	body.append(tmpData.m_Filename, tmpData.m_FileNameLen);

	int ret = sendRequest(body, result);
	if(ret)
	{
		LOG("send request error, ret:[%d]!\n", ret);
		return -2;
	}

	const char *psBuf = result.c_str();
	int32_t Opt = *(int32_t *)psBuf;
	psBuf += 4;
	if(Opt != tmpData.m_OperateType)
	{
		LOG("Opt error,Send [%d]--Recv [%d]\n", Opt, tmpData.m_OperateType);
		return -3;
	}

	int32_t NO = *(int32_t *)psBuf;
	psBuf += 4;
	if(NO != tmpData.m_NO)
	{
		LOG("NO error,Send [%d]--Recv [%d]\n", NO, tmpData.m_NO);
		return -4;
	}

	tmpData.m_Result = *(uint32_t *)psBuf;
	psBuf += 4;

	if(tmpData.m_Result < 0)
	{
		LOG("md5 error!,ret:[%d]\n", tmpData.m_Result);
		return tmpData.m_Result;
	}
	printf("result:[%d]\n", tmpData.m_Result);
	
	strmd5 = string(psBuf, 32);
	//memcpy(pData->m_Listname, psBuf, 32);
	//pData->m_Listname[32] = 0;
	//cout<<"md5:"<<strmd5<<endl;

	return 0;
}

int WorkClient::buildDataToStr(_DFIELD_* pData, string& result)
{

	if(pData->m_OperateType == DF_NOT_OPR)
	{
		LOG("Operate Type error: [%d]\n", pData->m_OperateType);
		return -1;
	}

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);


	//Have List
	if(pData->m_ListNameLen > 0)
	{
		//printf("append List:%d\t%s\n", pData->m_ListNameLen, pData->m_Listname);
		result.append((const char*)&pData->m_ListNameLen, 4);
		result.append(pData->m_Listname, pData->m_ListNameLen);
	}
	
	//Have File
	if(pData->m_FileNameLen > 0)
	{
		//printf("append File:%d\t%s\n", pData->m_FileNameLen, pData->m_Filename);
		result.append((const char*)&pData->m_FileNameLen, 4);
		result.append(pData->m_Filename, pData->m_FileNameLen);
	}

	if(pData->m_OperateType == DF_CLOSE_FILE)
	{
		result.append((const char*)&pData->m_DataLength, 4); //1:All, 0:Mod
	}
	else
	{
		//Have Data
		if(pData->m_DataLength > 0)
		{
			//printf("append Data:%d\n", pData->m_DataLength);
			result.append((const char*)&pData->m_DataLength, 4);
			result.append(pData->m_PData, pData->m_DataLength);
		}
	}

	return 0;
}

int WorkClient::renameFile(const char* pSrcName, const char* pDesName)
{
    assert(pSrcName != NULL && pDesName != NULL);

    _DFIELD_ tmpData;

    strcpy(tmpData.m_Listname, pSrcName);
	strcpy(tmpData.m_Filename, pDesName);
	tmpData.m_OperateType = DF_RENAME_FILE;
	tmpData.m_ListNameLen = strlen(tmpData.m_Listname);
    tmpData.m_FileNameLen = strlen(tmpData.m_Filename);
	tmpData.m_NO          = tmpData.m_FileNameLen;
	tmpData.m_DataLength  = 0;
	tmpData.m_OPNO        = 0;

	return send(&tmpData);
    return 0;
}

int WorkClient::createOneFile(const char* psCreateName, char* psData, const int iDataSize)
{
	assert(psCreateName!=NULL);

	_DFIELD_ tmpData;

	strcpy(tmpData.m_Filename, psCreateName);
	tmpData.m_OperateType = DF_CREATE_FILE;
	tmpData.m_FileNameLen = strlen(tmpData.m_Filename);
	tmpData.m_NO          = tmpData.m_FileNameLen;
	tmpData.m_ListNameLen = 0;
	tmpData.m_DataLength  = 0;
	tmpData.m_OPNO        = 0;

	if(psData != NULL && iDataSize > 0)
	{
		tmpData.m_DataLength = iDataSize;
		tmpData.m_PData      = psData;
	}

	return send(&tmpData);

}

int WorkClient::sendFileofList(char* psListName, char* psFileName, char* psRemotePath)
{
	char szRemoteList[300];
	char szRemoteFile[300];
	char szRealName[300];
	int  ret = 0;

	char* p = strrchr(psFileName, '/');
	if(p != NULL)
		strcpy(szRealName, p + 1);  //获取纯文件名
	else
		strcpy(szRealName, psFileName);

	if(psRemotePath != NULL)
	{
		sprintf(szRemoteList, "%s/%s", psRemotePath, psListName);
		sprintf(szRemoteFile, "%s/%s", psRemotePath, szRealName);
	}
	else
	{
		sprintf(szRemoteList, "%s", psListName);
		sprintf(szRemoteFile, "%s", szRealName);
	}

	char* pBuf = new char[MAX_SEND_BUF];

	_DFIELD_ TmpData;
	FILE* fpr = fopen(psFileName, "rb");
	if(!fpr)
	{
		LOG("Open File %s error, errno:[%d]\n", psFileName, errno);
		delete[] pBuf;
		return -1;
	}

	strcpy(TmpData.m_Listname, szRemoteList);
	strcpy(TmpData.m_Filename, szRemoteFile);
	TmpData.m_OperateType = DF_OPEN_FILE;
	TmpData.m_ListNameLen = strlen(TmpData.m_Listname);
	TmpData.m_FileNameLen = strlen(TmpData.m_Filename);
	TmpData.m_NO          = TmpData.m_FileNameLen;
	TmpData.m_OPNO        = 0;
	TmpData.m_DataLength  = 0;
	TmpData.m_PData       = pBuf;

	ret = send(&TmpData);
	if(ret)
	{
		LOG("Send error!\n");
		delete [] pBuf;
		return ret;
	}

	while(!feof(fpr))
	{

		TmpData.m_OperateType = DF_WRITE_FILE;
		TmpData.m_ListNameLen = 0;
		TmpData.m_DataLength  = fread(TmpData.m_PData, 1, MAX_SEND_BUF, fpr);
		if(TmpData.m_DataLength <= 0)
			break;

		ret = send(&TmpData);
		if(ret)
		{
			LOG("Send error!\n");
			break;
		}

	}

	//Close File
	//printf("close file\n");

	
	TmpData.m_OperateType = DF_CLOSE_FILE; 
	TmpData.m_DataLength  = 1;  //All

	ret = send(&TmpData);
	if(ret)
	{
		LOG("Send error!\n");
		delete[] pBuf;
		return ret;
	}

	string strresult;
	ret = buildMd5(psFileName, strresult, pBuf);
	if(ret)
	{
		LOG("build %s error, ret:[%d]\n", psFileName, ret);
	}
	else
	{
		ret = checkMD5(psFileName, szRemoteFile);
		if(ret)
		{
			LOG("check %s error, ret:[%d]\n", psFileName, ret);
		}
	}
	
	delete[] pBuf;

	return ret;
}

int WorkClient::sendList(const char* psListName, char* psRemotePath)
{
	assert(psListName != NULL);

	_DFIELD_ TmpData;
	int ret = 0;
	int  iLen = 0;
	
	char szListName[300];
	char szRemoteList[300];

	char* p = const_cast<char *>(strrchr(psListName, '/'));
	if(p != NULL)
		strcpy(szListName, p+1);  //获取纯文件名
	else
		strcpy(szListName, psListName);

	if(psRemotePath == NULL)
	{
		sprintf(szRemoteList, "%s", szListName);
	}
	else
	{
		iLen = strlen(psRemotePath);
		if(psRemotePath[iLen - 1] == '/')
			psRemotePath[iLen - 1] = 0;

		sprintf(szRemoteList, "%s/%s", psRemotePath, szListName);
	}


	FILE* fpr = fopen(psListName, "rb");
	if(!fpr)
	{
		LOG("Open File %s error, errno:[%d]\n", psListName, errno);
		return -1;
	}

	
	strcpy(TmpData.m_Listname, szRemoteList);
	TmpData.m_OperateType = DF_START_LST;
	TmpData.m_ListNameLen = strlen(TmpData.m_Listname);
	TmpData.m_NO          = TmpData.m_ListNameLen;
	TmpData.m_OPNO        = 0;
	TmpData.m_FileNameLen = 0;
	TmpData.m_DataLength  = 0;
	TmpData.m_PData       = NULL;


	ret = send(&TmpData);
	if(ret)
	{
		LOG("Send error!\n");
		return ret;
	}

	char szFileName[300];
	while(fgets(szFileName, 300, fpr) != NULL)
	{

		iLen = strlen(szFileName);
		if(szFileName[iLen - 1] == '\n')
			szFileName[iLen - 1] = 0;

		ret = sendFileofList(szListName, szFileName, psRemotePath);
		if(ret)
		{
			LOG("Send error!\n");
			break;
		}
		printf("Send:%s\n", szFileName);
	}

	fclose(fpr);

	//Close List
	TmpData.m_OperateType = DF_END_LST; 
	ret = send(&TmpData);
	if(ret)
	{
		LOG("Send error!\n");
		return ret;
	}

	return 0;
}

int WorkClient::sendFile(const char* psSendName, char* psRemotePath)
{
	assert(psSendName != NULL);

	char* pBuf = new char[MAX_SEND_BUF];
	_DFIELD_ TmpData;
	int ret = 0;
	int iLen = 0;
	char szRemoteFile[300];
	char szRealFile[300];

	
	char* p = const_cast<char *>(strrchr(psSendName, '/'));
	if(p != NULL)
		strcpy(szRealFile, p + 1);  //获取纯文件名
	else
		strcpy(szRealFile, psSendName);

	if(psRemotePath != NULL)
	{
		iLen = strlen(psRemotePath);
		if(psRemotePath[iLen - 1] == '/')
			psRemotePath[iLen - 1] = 0;

		sprintf(szRemoteFile, "%s/%s", psRemotePath, szRealFile);
	}
	else
	{
		sprintf(szRemoteFile, "%s", szRealFile);

	}
	

	FILE* fpr = fopen(psSendName, "rb");
	if(!fpr)
	{
		LOG("Open File %s error, errno:[%d]\n", psSendName, errno);
		delete[] pBuf;
		return -1;
	}

	strcpy(TmpData.m_Filename, szRemoteFile);
	TmpData.m_OperateType = DF_SINGLE_FILE;
	TmpData.m_FileNameLen = strlen(TmpData.m_Filename);
	TmpData.m_NO          = TmpData.m_FileNameLen;
	TmpData.m_OPNO        = 0;
	TmpData.m_ListNameLen = 0;
	TmpData.m_DataLength  = 0;
	TmpData.m_PData       = pBuf;
	
	ret = send(&TmpData);
	if(ret)
	{
		LOG("Send error!\n");
		delete [] pBuf;
		return ret;
	}


	while(!feof(fpr))
	{
		TmpData.m_OperateType = DF_WRITE_FILE;
		TmpData.m_DataLength = fread(TmpData.m_PData, 1, MAX_SEND_BUF, fpr);
		
		if(TmpData.m_DataLength <= 0)
			break;

		ret = send(&TmpData);
		if(ret)
		{
			LOG("Send error!\n");
			break;
		}
	}

	fclose(fpr);

	//Close File
	
	TmpData.m_OperateType = DF_CLOSE_FILE; 
	TmpData.m_DataLength  = 1; //All

	ret = send(&TmpData);
	if(ret)
	{
		LOG("Send error!\n");
		delete [] pBuf;
		return ret;
	}

	string strresult;
	ret = buildMd5(psSendName, strresult, pBuf);
	if(ret)
	{
		LOG("build %s fail, ret:[%d]\n", psSendName, ret);
	}
	else
	{
		ret = checkMD5(psSendName, szRemoteFile);
		if(ret)
		{
			LOG("check fail, ret:[%d]\n");
		}

	}

	delete [] pBuf;
	return ret;
}

int WorkClient::checkOneFile(const char* psCheckName)
{
	return commonWork(psCheckName, DF_CHECK_FILE);
}

int WorkClient::removeOneFile(const char* psRmFileName)
{
	return commonWork(psRmFileName, DF_DEL_FILE);
}

int WorkClient::chmodFile(const char* psFileName)
{
	return commonWork(psFileName, DF_FILE_CHMOD);
}

int WorkClient::killProcess(const char* psFileName)
{
	return commonWork(psFileName, DF_KILL_PROCESS);
}

int WorkClient::execProcess(const char* psFileName)
{
	return commonWork(psFileName, DF_EXEC_PROCESS);
}

int WorkClient::RPCShell(const char* shell, string &strres)
{
	assert(shell != NULL);

	_DFIELD_ tmpData;

	strcpy(tmpData.m_Filename, shell);
	tmpData.m_OperateType = DF_RPC_SHELL;
	tmpData.m_FileNameLen = strlen(tmpData.m_Filename);
	tmpData.m_NO          = tmpData.m_FileNameLen;
	tmpData.m_ListNameLen = 0;
	tmpData.m_DataLength  = 0;
	tmpData.m_OPNO        = 0;

	int ret = 0;
	string body, result;

	ret = buildDataToStr(&tmpData, body);
	if(ret)
	{
		LOG("Build String error!, ret:[%d]\n", ret);
		return ret;
	}

	ret = sendRequest(body, result);
	if(ret)
	{
		LOG("Send Data error, ret:[%d]\n", ret);
		return ret;
	}

	const char *psBuf = result.c_str();
	int32_t Opt = *(int32_t *)psBuf;
	psBuf += 4;
	if(Opt != tmpData.m_OperateType)
	{
		LOG("Opt error,Send [%d]--Recv [%d]\n", Opt, tmpData.m_OperateType);
		return -3;
	}

	int32_t NO = *(int32_t *)psBuf;
	psBuf += 4;
	if(NO != tmpData.m_NO)
	{
		LOG("NO error,Send [%d]--Recv [%d]\n", NO, tmpData.m_NO);
		return -4;
	}

	tmpData.m_Result = *(uint32_t *)psBuf;
	psBuf += 4;

	int32_t count = *(int32_t *)psBuf;
	psBuf += 4;

	strres = string(psBuf, count);

	return 0;
}

