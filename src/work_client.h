/**
 * @file workclient.h
 * @brief 通用客户端
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-08
 */

#ifndef _WORK_CLIENT_H_
#define _WORK_CLIENT_H

#include <iostream>

#include "QSEPSDF_CS.h"
#include "md5.h"
#include "protocol.h"

using namespace std;

typedef struct stIPPort
{
	string           m_strIP;
	uint32_t         m_iPort;
}_IPPORT_;

class WorkClient
{
	public:
		WorkClient();
		WorkClient(const string& strIP, const uint32_t iPort);
		~WorkClient();

	public:

		void     setIP(const string& strIP)   { m_strServerIP = strIP;}
		void     setPort(const uint32_t iPort){ m_iServerPort = iPort;}

		string   getIP() const  { return m_strServerIP;}
		uint32_t getPort() const{ return m_iServerPort;}

		//common function
		int Initialize();
		void close();
		int parseResult(string& result, _DFIELD_* pData);
		int send(_DFIELD_* pData);
		int sendRequest(const string& body, string& result);
		int buildDataToStr(_DFIELD_* pData, string& result);
		int commonWork(const char* file, int ioper);

		// about MD5
		int buildMd5(const char* szFile, string& strmd5, char* pBuf);
		int checkMD5(const char* psLocalFile, const char* psRemoteFile);
		int getMD5(const char* psFileName, string& strmd5);

		//File
        int renameFile(const char* pSrcName, const char* pDesName);
		int createOneFile(const char* psCreateName, char* psData, const int iDataSize);
		int checkOneFile(const char* psCheckName);
		int removeOneFile(const char* psRmFileName);
		int sendFile(const char* psSendName, char* psRemotePath);

		//List
		int sendFileofList(char* psListName, char* psFileName, char* psRemotePath);
		int sendList(const char* psListName, char* psRemotePath);

		//Process
		int chmodFile(const char* psFileName);
		int killProcess(const char* psFileName); //File name has no path
		int execProcess(const char* psFileName);

		int RPCShell(const char* shell, string &strres);

	private:
		//stIPPort  m_stIPPort;
		TCPClient* m_tcpclient;
		string     m_strServerIP;
		uint32_t   m_iServerPort;
};
#endif  //end of work_client
