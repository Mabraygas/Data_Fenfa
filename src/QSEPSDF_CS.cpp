/**
 * @file CQSEPSDF_CS.cpp
 * @brief 分发客户端
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-09-24
 */

#include <string>
#include <map>
#include <iostream>
#include <sys/stat.h>
#include <net/if.h>

#include <tinyxml.h>

#include "protocol.h"
#include "QSEPSDF_CS.h"
#include "DayLog.h"
#include "SmsNotify.h"
#include "md5.h"
#include "work_client.h"

#define  LOG(format, args...) g_clilog.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)
//#define  LOGP(format, args...) g_parselog.TWrite(format,##args)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;

DayLog g_clilog;
DayLog g_parselog;
DayLog g_log;

CQSEPSDF_CS::CQSEPSDF_CS()
{
	m_LstNum = MAX_LST_NUM;
    m_FileNum = FILE_NUM;
	m_ThreadNO = 0;
    m_LstInf = new _LST_INF_[MAX_LST_NUM];
    m_FileInf = new _SF_INF_[FILE_NUM];
    m_pDataQue = new QueEvent***[MAX_LST_NUM];
    for(int p = 0 ; p < MAX_LST_NUM ; p ++ ) {
        m_pDataQue[p] = new QueEvent**[MAX_GROUP_NUM];
        for(int q = 0 ; q < MAX_GROUP_NUM ; q ++ ) {
            m_pDataQue[p][q] = new QueEvent*[MAX_S_NUM_AGROUP];
        }
    }
}

CQSEPSDF_CS::~CQSEPSDF_CS()
{
	DeleteCriticalSection(&m_MainCri);
	delete m_MainControl;
    delete[] m_LstInf;
    delete[] m_FileInf;
    for(int p = 0 ; p < MAX_LST_NUM ; p ++ ) {
        for(int q = 0 ; q < MAX_GROUP_NUM ; q ++ ) {
            delete []m_pDataQue[p][q];
        }
        delete []m_pDataQue[p];
    }
    delete []m_pDataQue;
}

int CQSEPSDF_CS::getLocalIP(char* outip)
{
	int i=0;
	int sockfd;
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	char* ip;
	//初始化ifconf
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}

	ioctl(sockfd, SIOCGIFCONF, &ifconf); //获取所有接口信息
	close(sockfd);

	//接下来一个一个的获取IP地址
	ifreq = (struct ifreq*)buf;
	for( i = (ifconf.ifc_len/sizeof(struct ifreq)); i > 0; i++)
	{
		ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);

		if(strcmp(ip,"127.0.0.1")==0) //排除127.0.0.1，继续下一个
		{
			ifreq++;
			continue;
		}
		strcpy(outip,ip);
		return 0;
	}

	return -1;

}

int CQSEPSDF_CS::InitSys(CQSEPSDF_MainControl* pMain)
{
	m_MainControl = pMain;

	/*if(m_MainControl->getDataPath() != NULL)
		strcpy(m_szRoot, m_MainControl->getDataPath());
	else
		sprintf(m_szRoot, "/opt/sokuproc/data");
	 */
	g_clilog.Init(30, "../log/DFS_Client");
	//g_parselog.Init(30, "../log/DFS_Parse");
	g_log.Init(30, "../log/data_verify");

	InitializeCriticalSection(&m_MainCri);

	int Ret =LoadAllINF(); 
	if (Ret)
	{
		LOG("Load Info error, return %d\n", Ret);
		return Ret; 
	}
	int i(0),j(0),k(0),p(0),q(0);
	for(i = 0; i < m_LstNum; i++)
	{
		for(j = 0; j < m_LstInf[i].m_RealGroupNum; j++)
		{
			for(k = 0; k < m_LstInf[i].m_GroupInf[j].m_RealServerNum; k++)
			{
				m_pDataQue[i][j][k] = new QueEvent(m_MainControl->getQueSize()); 
			}
	
		}
	}

	m_MainControl->ReadLstNO(m_LstNum);
	getLocalIP(m_szLocalIp);
	
	LOG("===========================DFS client Initializes successfully!===========================\n");

	return 0;
}

/*
*/
int CQSEPSDF_CS::LoadAllINF()
{
   int i,j,k,p,q;
   for (i=0; i<MAX_LST_NUM; i++)
   {
	    m_LstInf[i].m_RealGroupNum = 0;
		m_LstInf[i].m_CheckLstMName[0] = 0;
		m_LstInf[i].m_CurLstKey =  0;
		m_LstInf[i].m_CurFileFP = NULL;
		*m_LstInf[i].m_GroupRoot = 0;
		*m_LstInf[i].m_GroupLstName = 0;

	   for (j=0; j<MAX_GROUP_NUM; j++)
	   {
		   m_LstInf[i].m_GroupInf[j].m_CurCheckKey = 0;
		   m_LstInf[i].m_GroupInf[j].m_GroupNo = j;
		   m_LstInf[i].m_GroupInf[j].m_RealServerNum = 0;
		   m_LstInf[i].m_GroupInf[j].m_SendDataType = 256;
           *m_LstInf[i].m_GroupInf[j].m_GroupRoot = 0;
           *m_LstInf[i].m_GroupInf[j].m_GroupLstName = 0;
		  
		   for (k=0; k<MAX_S_NUM_AGROUP; k++)
		   {
			   m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerNO = k;
			   m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerHand = NULL;
			   *m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerIPStr = 0;
			   m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerPort = 0;   
		   }
	   }
   }
   //TODO 读取信息

   const char * xmlFile = "soku.xml";	
   int ret = 0;
   while(1){
       if(access(xmlFile, 0))
       {
           printf("Please load the xml file:%s, wait 60 seconds\n", xmlFile);
           sleep(60);
           continue;
       }
       ret = LoadXml(xmlFile);
       if(ret)
       {
           LOG("Load xml file error, return %d\n", ret);
           sleep(60);
           continue;
       }
       break;
   }
   return 0;
}

/*
*/
int CQSEPSDF_CS::LoadXml(const char* xmlFile)
{
   if(xmlFile == NULL)
   {
       printf("xml name is null\n");
       return -1;
   }
   int i(0),j(0),k(0),p(0),q(0);
   int file_find = 0;
   TiXmlDocument doc;  
   if (!doc.LoadFile(xmlFile)) {  	
	   LOG("can not parse xml %s\n", xmlFile);
	   return -2;
   }

   TiXmlElement* rootElement = doc.RootElement();  //Soku元素  
   TiXmlAttribute* attributeOfSoku = rootElement->FirstAttribute();
   TiXmlAttribute* attributeOfSokuNext = attributeOfSoku->Next();

   if(strcmp("ListNumber", attributeOfSoku->Name()) == 0)
	   m_LstNum = attributeOfSoku->IntValue();
   if(strcmp("FileNumber", attributeOfSokuNext->Name()) == 0)
       m_FileNum = attributeOfSokuNext->IntValue();


   TiXmlElement* ListElement = rootElement->FirstChildElement();  //List  

   for (i = 0; ListElement != NULL; ListElement = ListElement->NextSiblingElement(),i++ ) {  
	   file_find = 0;
       TiXmlAttribute* attributeOfList = ListElement->FirstAttribute(); 
	   for (;attributeOfList != NULL; attributeOfList = attributeOfList->Next() ) {  
		   if(strcmp(attributeOfList->Name(), "Source") == 0) {
                file_find = 1;
                break;
           }
           else if(strcmp(attributeOfList->Name(), "FileRoot") == 0) {
                
           }
           else if(strcmp(attributeOfList->Name(), "name") == 0)
		   {
			   strcpy(m_LstInf[i].m_CheckLstMName, attributeOfList->Value());

		   }else if(strcmp(attributeOfList->Name(), "key") == 0)
		   {
			   m_LstInf[i].m_CurLstKey = attributeOfList->IntValue();
		   }else if(strcmp(attributeOfList->Name(), "GroupRoot") == 0)
		   {
			   strcpy(m_LstInf[i].m_GroupRoot, attributeOfList->Value());
			   int iLen = strlen(m_LstInf[i].m_GroupRoot);
			   if(m_LstInf[i].m_GroupRoot[iLen-1] == '/')
				   m_LstInf[i].m_GroupRoot[iLen-1] = '\0';
		   }else if(strcmp(attributeOfList->Name(), "GroupListName") == 0)
		   {
			   strcpy(m_LstInf[i].m_GroupLstName, attributeOfList->Value());
		   }else if(strcmp(attributeOfList->Name(), "GroupNumber") == 0)
		   {
			   m_LstInf[i].m_RealGroupNum = attributeOfList->IntValue();
		   }
	   }
        if(file_find) 
            break;

	   TiXmlElement* GroupElement = ListElement->FirstChildElement();//Group 
	   for (j = 0; GroupElement != NULL; GroupElement = GroupElement->NextSiblingElement(), j++ ) {
		   TiXmlAttribute* attributeOfGroup = GroupElement->FirstAttribute(); 
		   for (;attributeOfGroup != NULL; attributeOfGroup = attributeOfGroup->Next() ) {  
			   if(strcmp(attributeOfGroup->Name(), "GroupNo") == 0)
			   {
				   m_LstInf[i].m_GroupInf[j].m_GroupNo = attributeOfGroup->IntValue();
			   }else if(strcmp(attributeOfGroup->Name(), "SendDataType") == 0)
			   {
				   m_LstInf[i].m_GroupInf[j].m_SendDataType = attributeOfGroup->IntValue();
			   }else if(strcmp(attributeOfGroup->Name(), "ServerNumber") == 0)
			   {
				   m_LstInf[i].m_GroupInf[j].m_RealServerNum = attributeOfGroup->IntValue();
			   }else if(strcmp(attributeOfGroup->Name(), "GroupRoot") == 0)
               {
                   strcpy(m_LstInf[i].m_GroupInf[j].m_GroupRoot, attributeOfGroup->Value());
                   int jLen = strlen(m_LstInf[i].m_GroupInf[j].m_GroupRoot);
                   if(m_LstInf[i].m_GroupInf[j].m_GroupRoot[jLen - 1] == '/')
                       m_LstInf[i].m_GroupInf[j].m_GroupRoot[jLen - 1] = '\0';
               }else if(strcmp(attributeOfGroup->Name(), "GroupListName") == 0)
               {
                   strcpy(m_LstInf[i].m_GroupInf[j].m_GroupLstName, attributeOfGroup->Value());
               }
		   }


		   TiXmlElement* ServerElement = GroupElement->FirstChildElement();//Server 
		   for (k = 0; ServerElement != NULL; ServerElement = ServerElement->NextSiblingElement(), k++ ) {
			   TiXmlAttribute* attributeOfServer = ServerElement->FirstAttribute(); 
			   for (;attributeOfServer != NULL; attributeOfServer = attributeOfServer->Next() ) {  
				   if(strcmp(attributeOfServer->Name(),"ServerNo") == 0)
				   {
					   m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerNO = attributeOfServer->IntValue();
				   }else if(strcmp(attributeOfServer->Name(),"IP") == 0)
				   {
					   strcpy(m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerIPStr, attributeOfServer->Value());
				   }else if(strcmp(attributeOfServer->Name(),"Port") == 0)
				   {
					   m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerPort = attributeOfServer->IntValue();
				   }

			   }

		   }

		   m_LstInf[i].m_GroupInf[j].m_RealServerNum = k;

	   }
	   m_LstInf[i].m_RealGroupNum  = j;

   } 
   m_LstNum = i;

    for(p = 0; ListElement != NULL; ListElement = ListElement->NextSiblingElement(), p++) {
        TiXmlAttribute* attributeOfFile = ListElement->FirstAttribute();
        for(; attributeOfFile != NULL; attributeOfFile = attributeOfFile->Next()) {
            if(strcmp(attributeOfFile->Name(), "Source") == 0) {
                ;
            }
            else if(strcmp(attributeOfFile->Name(), "FileRoot") == 0) {
                ;
            }
            else if(strcmp(attributeOfFile->Name(), "SendDataType") == 0) {
                ;
            }
            else if(strcmp(attributeOfFile->Name(), "ServerNumber") == 0) {
                ;
            }
        }

        TiXmlElement* ServerElememt = ListElement->FirstChildElement();
        for(q = 0; ServerElememt != NULL; ServerElememt = ServerElememt->NextSiblingElement(), q++) {
            TiXmlAttribute* attributeOfServer = ServerElememt->FirstAttribute();
            for(; attributeOfServer != NULL; attributeOfServer = attributeOfServer->Next()) {
                if(strcmp(attributeOfServer->Name(), "ServerNo") == 0) {
                    ;
                }
                else if(strcmp(attributeOfServer->Name(), "IP") == 0) {
                    ;
                }
                else if(strcmp(attributeOfServer->Name(), "Port") == 0) {
                    ;
                }
            }
        }
    }
    
   return 0;
}


//读到下一条记录 0
//文件结束 -1
int CQSEPSDF_CS::ReadAfieldFromFile(_DFIELD_* pData)
{	
	char g_sLine[1024];
	uint64_t ddwKey;

	int iLen = strlen(pData->m_PData);
	strcpy(g_sLine, pData->m_PData);

	*(g_sLine + iLen - 1) = '\0';
	
	
	char * apSep[20] = {0};
	char * p    = g_sLine;
	int iCont   = 0;
	char * p1   = p;
	while (*p && iCont < 20) {
		while (*p && *p!='\t')
		{ // Field/Column 以 tab 分隔
			p++;
		}
		apSep[iCont] = p1;

		if (*p==0) {
			break;
		}
		iCont++;
		*p = 0;
		p++;
		p1 =p;
	}
	if (iCont != 14 && iCont != 17) { // 只兼容 15,18 这两种字段
		*(pData->m_PData + iLen - 1) = '\0';
		LOG("%s:[%d] '%s'\r\n", pData->m_Filename, iCont, pData->m_PData);
		return -3;
	}

	ddwKey = atoll(apSep[2]); //读取Video ID
	if (ddwKey < 1 || ddwKey > 0x7FFFFFFF) {
		*(pData->m_PData + iLen - 1) = '\0';
		LOG("%s: [%d] '%s'\n", pData->m_Filename, ddwKey, pData->m_PData);
		return -4;
	}

	pData->m_Key        = ddwKey;
	pData->m_DataSize   = iLen + 1;
	pData->m_DataLength = iLen;

	//memcpy(pData->m_PData, sLine, pData->m_DataSize);
	return 0;
}

int CQSEPSDF_CS::SendAData(std::string& result, _DFIELD_* pData, ServerInf* pServer)
{	
	if(result.empty() || pServer == NULL)
	{
		LOG("parameter is null\n");
		return -10;
	}
	//Build head
	Protocol head;	
	head.body_len = result.size();	


	//Build package
	string msg;	
	Protocol::HeadToBuffer(head,msg);	
	msg += result;

	//send package
	string err;
	int ret = 0, iLoop = 0;
	do{
		ret = pServer->m_ServerHand->send(msg, err);
		if(ret == 0)
			break;
		if(ret < 0){             
			// send message
			//printf("Send:%s\n", err.c_str());
			if(iLoop++ > 20);
			{
				LOG("send failed:%s\n",err.c_str());
				char szMsg[300];
				snprintf(szMsg, sizeof(szMsg), "[%s Error]: Send Fail,Server[%s:%d]!", m_szLocalIp, 
						pServer->m_ServerIPStr, pServer->m_ServerPort);
				SendSmsNotify(szMsg);
				iLoop = 0;
			}
			usleep(100);
		}   
	}while(ret < 0);


	//receive data
	char recvBuf[1024];
	//memset(recvBuf, 0, sizeof(recvBuf));

	size_t len = 33;
	ret = pServer->m_ServerHand->recv(recvBuf, len, err);
	if(ret < 0)
		return ret;

	//parsing recv data
	string o;
	string strBuf = string(recvBuf, len);
	if(Protocol::ParseProtocol(strBuf, o) != PACKET_FULL)
	{
		LOG("Package error\n");
		return -1;
	}

	if(o.length() < 12)
	{
		LOG("Package body length error[%d]\n", o.length());
		return -2;
	}
	
	const char *psBuf = o.c_str();
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
		int8_t ierrno = *(int8_t *)psBuf;
		LOG("Send Data Error, errno:[%d]\n", ierrno);
		return -5;
	}
	return 0;

}

int CQSEPSDF_CS::CreateALst(_DFIELD_*pData, ServerInf* pServer)
{
	string result = "";

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	result.append((const char*)&pData->m_ListNameLen, 4);
	result.append(pData->m_Listname, pData->m_ListNameLen);

#ifdef NLOG
	//printf("%s:%s\n", __FUNCTION__, pData->m_Listname);
#endif

	int SendResult = -1;
	int iLoop = 0;

	do{
		SendResult= SendAData(result, pData, pServer);
		if(SendResult)
		{
			LOG("Sleep: ret[%d], Remote [%s:%d]\n", SendResult, pServer->m_ServerIPStr, pServer->m_ServerPort);
			if(iLoop++ > 3)
			{
				char szMsg[300];
				snprintf(szMsg, sizeof(szMsg), 
						"[%s SL]:Sleep[%d],Server[%s:%d]!", 
						m_szLocalIp, 
						SendResult,
						pServer->m_ServerIPStr, 
						pServer->m_ServerPort);
				SendSmsNotify(szMsg);
				break;
			}
			Sleep(1);
		}
	}while(SendResult);

	LOG("Create List[%s:%d]: %s\n", pServer->m_ServerIPStr, pServer->m_ServerPort, pData->m_Listname);
	m_MainControl->FreeAField(pData);

	return 0;

}
int CQSEPSDF_CS::EndALst(_DFIELD_ *pData,ServerInf * pServer)
{
	string result;

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	result.append((const char*)&pData->m_ListNameLen, 4);
	result.append(pData->m_Listname, pData->m_ListNameLen);

    int SendResult= -1;
	int iLoop = 0;
	do 
	{
		SendResult = SendAData(result, pData, pServer);
		if (SendResult < 0)
		{
			LOG("Sleep: ret[%d], Remote [%s:%d]\n", SendResult, pServer->m_ServerIPStr, pServer->m_ServerPort);
			if(iLoop++ > 3)
			{
				char szMsg[300];
				snprintf(szMsg, sizeof(szMsg), 
						"[%s CL]:Sleep[%d],Server[%s:%d]!", 
						m_szLocalIp, 
						SendResult,
						pServer->m_ServerIPStr, 
						pServer->m_ServerPort);
				SendSmsNotify(szMsg);
				break;
			}
			Sleep(1);
		}
	} while (SendResult < 0);

	//关闭连接
	pServer->m_ServerHand->close();
#ifdef NLOG
	//cout<<"End List:"<<pData->m_Listname<<endl;
#endif
	LOG("Close List[%s:%d]:%s\n", pServer->m_ServerIPStr, pServer->m_ServerPort, pData->m_Listname);

	m_MainControl->FreeAField(pData);
	
	return 0;
}

int CQSEPSDF_CS::OpenAFile(_DFIELD_ *pData,ServerInf * pServer)
{
	string result;

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	result.append((const char*)&pData->m_ListNameLen, 4);
	result.append(pData->m_Listname, pData->m_ListNameLen);
	result.append((const char*)&pData->m_FileNameLen, 4);
	result.append(pData->m_Filename, pData->m_FileNameLen);

	//LOG("[%s:%d] List:%s\tFile:%s\n", Server->m_ServerIPStr, Server->m_ServerPort, pData->m_PData, pData->m_Filename);
	//printf("DataSize:%d,File:%s\nList:%s\n", iDataSize, pData->m_Filename, pData->m_PData);
	int SendResult= -1;
	int iLoop = 0;
	do 
	{
		SendResult = SendAData(result, pData, pServer);
		if (SendResult)
		{
			LOG("Sleep: ret[%d], Remote [%s:%d]\n", SendResult, pServer->m_ServerIPStr, pServer->m_ServerPort);
			if(iLoop++ > 3)
			{
				char szMsg[300];
				snprintf(szMsg, sizeof(szMsg), 
						"[%s OF]:Sleep[%d],Server[%s:%d]!", 
						m_szLocalIp, 
						SendResult,
						pServer->m_ServerIPStr, 
						pServer->m_ServerPort);
				SendSmsNotify(szMsg);
				break;

			}
			Sleep(1);
		}
	} while (SendResult);

	m_MainControl->FreeAField(pData);

	return 0;
}

int CQSEPSDF_CS::CloseAFile(_DFIELD_ *pData, ServerInf * pServer)
{
	string result;

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	//result.append((const char*)&pData->m_ListNameLen, 4);
	//result.append(pData->m_Listname, pData->m_ListNameLen);
	result.append((const char*)&pData->m_FileNameLen, 4);
	result.append(pData->m_Filename, pData->m_FileNameLen);
	result.append((const char*)&pData->m_DataLength, 4);   //1 All, 0 Mod

	int SendResult= -1;
	int iLoop = 0;
	do 
	{
		SendResult = SendAData(result, pData, pServer);
		if (SendResult)
		{
			LOG("Sleep: ret[%d], Remote [%s:%d]\n", SendResult, pServer->m_ServerIPStr, pServer->m_ServerPort);
			if(iLoop++ > 3)
			{
				char szMsg[300];
				snprintf(szMsg, sizeof(szMsg), 
						"[%s CF]:Sleep[%d],Server[%s:%d]!", 
						m_szLocalIp, 
						SendResult,
						pServer->m_ServerIPStr, 
						pServer->m_ServerPort);
				SendSmsNotify(szMsg);
				break;

			}
			Sleep(1);
		}
	} while (SendResult);

	/*EnterCriticalSection(&m_MainCri);

	if(pData->m_FreeSymbole <= 1)
	{
		m_LstNO[pData->m_OPNO].m_FileNum++;
		UpdateLstNO(pData->m_OPNO);
	}

	LeaveCriticalSection(&m_MainCri);
	*/

	m_MainControl->FreeAField(pData);

	return 0;

}

int CQSEPSDF_CS::WriteADataToFile(_DFIELD_ *pData, ServerInf * pServer)
{
	string result = "";

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	//result.append((const char*)&pData->m_ListNameLen, 4);
	//result.append(pData->m_Listname, pData->m_ListNameLen);
	result.append((const char*)&pData->m_FileNameLen, 4);
	result.append(pData->m_Filename, pData->m_FileNameLen);
	result.append((const char*)&pData->m_DataLength, 4);
	result.append(pData->m_PData, pData->m_DataLength);

    int SendResult= -1;
	//int iLoop = 0;
	do 
	{
		SendResult = SendAData(result, pData, pServer);
		if (SendResult)
		{
			//if(iLoop++ > 200)
			//{
				//LOG("Sleep: ret[%d], Remote [%s:%d]\n", SendResult, pServer->m_ServerIPStr, pServer->m_ServerPort);
			//}
			Sleep(1);
		}
	} while (SendResult);

	if(pData->m_MemType == BIG)
		m_MainControl->FreeAField(pData);
	return 0;
}

int CQSEPSDF_CS::GetrealStr(char* source)
{
	int lLen = strlen(source) -1;
	while (lLen >0 &&(*(source +lLen) == 0 || *(source+lLen) == '\t' || *(source +lLen) == 0x0a||*(source+lLen) == 0x0d
		|| *(source+lLen) == ' '))
	{
		lLen --;
	}
	lLen ++;
	*(source +lLen) =0;
	
	return lLen;
}

int CQSEPSDF_CS::Work_StartList(const int32_t CurThreadNo, const int pointer,_DFIELD_* pTmpData)
{
	int result = -1;
	int k = 0, j = 0;
	//for ( k = 0; k < m_LstInf[CurThreadNo].m_RealGroupNum; k++)
	//{

		_DFIELD_* pStartData;
		_DFIELD_ StartData;

		StartData.m_DataSize = 0;
		//StartData.m_Filename[0] = '\0';
		//StartData.m_FileNameLen = 0;
		StartData.m_MemType = DF_NOT_NEED_MEM;
		StartData.m_PData= NULL;

		//申请一个空间
		do 
		{
			result = m_MainControl->GetAFreeField(pStartData, StartData);
			if (result)
			{
				Sleep(1);
			}
		} while (result);
		
		pStartData->m_OperateType = DF_START_LST;
		pStartData->m_DataLength = 0;
		pStartData->m_FP  = NULL;
		pStartData->m_FreeSymbole = m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum;
		pStartData->m_OPNO = m_LstInf[CurThreadNo].m_CurLstKey;
		pStartData->m_Result = -1;
		pStartData->m_Key = 0;
		strcpy(pStartData->m_Listname, pTmpData->m_Listname);
		pStartData->m_ListNameLen = pTmpData->m_ListNameLen;  

		for (j =0; j < m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum; j++)
		{
			m_pDataQue[CurThreadNo][pointer][j]->Push(pStartData->m_NO);
		} 
		LOG("Work [%d] Servers Start List '%s' \n", j, pTmpData->m_Listname);

	//}
	return 0;
}

int CQSEPSDF_CS::Work_OpenFile(const int32_t CurThreadNo, const int pointer, _DFIELD_* pTmpData)
{
	int result = -1;

	//for (int k = 0; k < m_LstInf[CurThreadNo].m_RealGroupNum; k++)
	//{
		_DFIELD_* pOpenData;
		_DFIELD_ OpenData;

		OpenData.m_MemType = DF_NOT_NEED_MEM;
		OpenData.m_PData = NULL;
		OpenData.m_DataSize = 0;
		
		do 
		{
			result = m_MainControl->GetAFreeField(pOpenData, OpenData);
			if (result)
			{
				Sleep(1);
			}
		} while (result);

		pOpenData->m_OperateType = DF_OPEN_FILE;
		pOpenData->m_OPNO = m_LstInf[CurThreadNo].m_CurLstKey;
		pOpenData->m_Key = 0;

		strcpy(pOpenData->m_Filename, pTmpData->m_Filename);
		pOpenData->m_FileNameLen = pTmpData->m_FileNameLen;

		strcpy(pOpenData->m_Listname, pTmpData->m_Listname);
		pOpenData->m_ListNameLen = pTmpData->m_ListNameLen;

		pOpenData->m_DataLength = 0;
		pOpenData->m_FP = m_LstInf[CurThreadNo].m_CurFileFP;
		pOpenData->m_Result = -1;
		pOpenData->m_FreeSymbole = m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum;

		for (int j = 0; j < m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum; j++ )
		{
			m_pDataQue[CurThreadNo][pointer][j]->Push(pOpenData->m_NO);
		}
				  
	//}

	return 0;
}

int CQSEPSDF_CS::Work_CloseFile(const int32_t CurThreadNo, const int pointer, _DFIELD_* pTmpData)
{
	int result = -1;
	//for (int k =0; k<m_LstInf[CurThreadNo].m_RealGroupNum; k++)
	//{
		_DFIELD_ * pCloseFileData;
		_DFIELD_   CloseFileData;

		CloseFileData.m_MemType  = DF_NOT_NEED_MEM;//内存的类型
		CloseFileData.m_PData    = NULL;//数据存储的地址
		CloseFileData.m_DataSize = 0;//数据的大小

		do 
		{
			result = m_MainControl->GetAFreeField(pCloseFileData, CloseFileData);
			if (result)
			{
				Sleep(1);
			}
		} while (result);

		pCloseFileData->m_OperateType = DF_CLOSE_FILE;

		if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_SendDataType == DF_SEND_DATA_ALL)
			pCloseFileData->m_DataLength        = 1;
		else
			pCloseFileData->m_DataLength        = 0;

		//pCloseFileData->m_OPNO        = m_LstInf[CurThreadNo].m_CurLstKey;//LST的编号

		strcpy(pCloseFileData->m_Filename, pTmpData->m_Filename);
		pCloseFileData->m_FileNameLen = pTmpData->m_FileNameLen;

		/*
		strcpy(pCloseFileData->m_Listname, strMd5.c_str());
		pCloseFileData->m_ListNameLen = strMd5.size();
		*/

		//pCloseFileData->m_DataLength  = 0;//pTmpData->DataLength;
		pCloseFileData->m_FP          = m_LstInf[CurThreadNo].m_CurFileFP;//文件句柄
		pCloseFileData->m_Result      = -1;//操作是否成功的标志
		pCloseFileData->m_FreeSymbole = m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum;
		for (int j = 0; j < m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum; j++)
		{
			m_pDataQue[CurThreadNo][pointer][j]->Push(pCloseFileData->m_NO);
		}
				  
	//}//end for

	return 0;
}

int CQSEPSDF_CS::Work_CloseList(const int32_t CurThreadNo, const int pointer, _DFIELD_* pTmpData)
{
	int result = -1;
	int k = 0, j = 0;
	//for ( k = 0; k < m_LstInf[CurThreadNo].m_RealGroupNum; k++)
	//{
		_DFIELD_ *pEndData;
		_DFIELD_  EndData;

		EndData.m_MemType = DF_NOT_NEED_MEM;
		EndData.m_DataSize = 0;
		EndData.m_PData= NULL;

		
		//申请一个空间
		do 
		{
			result = m_MainControl->GetAFreeField(pEndData, EndData);
			if (result)
			{
				Sleep(1);
			}
		} while (result);
		  
		pEndData->m_OperateType = DF_END_LST;
		pEndData->m_DataLength = 0;
		pEndData->m_FP  =NULL;
		pEndData->m_OPNO = m_LstInf[CurThreadNo].m_CurLstKey;
		pEndData->m_Result = -1;
		pEndData->m_Key = 0;
		strcpy(pEndData->m_Listname, pTmpData->m_Listname);
		pEndData->m_ListNameLen = pTmpData->m_ListNameLen;
		pEndData->m_FreeSymbole = m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum;

		for (j = 0; j < m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum; j++)
		{
			m_pDataQue[CurThreadNo][pointer][j]->Push(pEndData->m_NO);
		} 

		LOG("Work [%d] Servers Close List '%s' \n", j, pTmpData->m_Listname);
		  
	//}

	return 0;
}

uint64_t CQSEPSDF_CS::Work_ModData(const int32_t CurThreadNo, const int GIdx, _DFIELD_* pTmpReadData, RStreamBuf* prstream)
{ 	//处理取模数据
	int k = GIdx;
	uint64_t ddwFileSize = 0;	
	int result = -1;

	while(prstream->getLine(pTmpReadData->m_PData, 1024) > 0) //按行读取
	{
		if(ReadAfieldFromFile(pTmpReadData))
			continue;

		_DFIELD_ * pReadData;
		_DFIELD_   ReadData;

		ReadData.m_PData = NULL;//数据存储的地址
		ReadData.m_MemType = SUITE;//小内存的类型
		ReadData.m_DataSize = pTmpReadData->m_DataSize;//数据的大小
		
		do 
		{
			result = m_MainControl->GetAFreeField(pReadData, ReadData);
			if (result)
			{
				Sleep(1);
			}

		} while (result);

		pReadData->m_OperateType = DF_WRITE_FILE;
		pReadData->m_OPNO        = m_LstInf[CurThreadNo].m_CurLstKey;//LST的编号
		pReadData->m_FP          = pTmpReadData->m_FP;               //文件句柄
		pReadData->m_Result      = -1;                               //操作是否成功的标志
		pReadData->m_FreeSymbole = 1;

		strcpy(pReadData->m_Filename, pTmpReadData->m_Filename);
		pReadData->m_FileNameLen = pTmpReadData->m_FileNameLen;

		//strcpy(pReadData->m_Listname, pTmpReadData->m_Listname);
		//pReadData->m_ListNameLen = pTmpReadData->m_ListNameLen;

		pReadData->m_DataLength  = pTmpReadData->m_DataLength;
		pReadData->m_Key         = pTmpReadData->m_Key;//关键Key

		ddwFileSize                += pReadData->m_DataLength;

		memcpy(pReadData->m_PData, pTmpReadData->m_PData, pTmpReadData->m_DataSize);

		int TmpServerNO1 = (pReadData->m_Key/4) % m_LstInf[CurThreadNo].m_GroupInf[k].m_RealServerNum;
		m_pDataQue[CurThreadNo][k][TmpServerNO1]->Push(pReadData->m_NO);

		//TmpLstNOINF.m_AllSize += TmpReadData.m_DataSize;

	}


	return ddwFileSize;
}

uint64_t CQSEPSDF_CS::Work_ALLData(const int32_t CurThreadNo, const int Gidx, _DFIELD_* pTmpReadData, FILE* fprData)
{   //处理复制数据
	int k = Gidx;
	int result = -1;
	uint64_t ddwFileSize = 0;

	while(!feof(fprData)) //按块读取
	{

		_DFIELD_ * pReadData;
		_DFIELD_   ReadData;
	
		pTmpReadData->m_DataLength = fread(pTmpReadData->m_PData, 1, 1048575, fprData);  //读取1MBytes-1大小的数据
		*(pTmpReadData->m_PData + pTmpReadData->m_DataLength) = '\0';
		pTmpReadData->m_DataSize = pTmpReadData->m_DataLength + 1;

		ReadData.m_PData = NULL;//数据存储的地址
		ReadData.m_MemType = BIG;//大内存的类型
		ReadData.m_DataSize = pTmpReadData->m_DataSize;

		do 
		{
			result = m_MainControl->GetAFreeField(pReadData, ReadData);
			if (result)
			{
				Sleep(1);
			}

		} while (result);

		pReadData->m_OperateType = DF_WRITE_FILE;
		pReadData->m_OPNO        = m_LstInf[CurThreadNo].m_CurLstKey;//LST的编号
		pReadData->m_Key         = 0;
		pReadData->m_FP          = pTmpReadData->m_FP;//文件句柄
		pReadData->m_Result      = -1;//操作是否成功的标志

		strcpy(pReadData->m_Filename, pTmpReadData->m_Filename);
		pReadData->m_FileNameLen = pTmpReadData->m_FileNameLen;

		//strcpy(pReadData->m_Listname, pTmpReadData->m_Listname);
		//pReadData->m_ListNameLen = pTmpReadData->m_ListNameLen;

		pReadData->m_FreeSymbole = m_LstInf[CurThreadNo].m_GroupInf[k].m_RealServerNum;

		pReadData->m_DataLength  = pTmpReadData->m_DataLength;
		ddwFileSize              += pReadData->m_DataLength;
		memcpy(pReadData->m_PData, pTmpReadData->m_PData, pTmpReadData->m_DataSize);

		for (int j = 0; j < m_LstInf[CurThreadNo].m_GroupInf[k].m_RealServerNum; j++)
		{
			m_pDataQue[CurThreadNo][k][j]->Push(pReadData->m_NO);
		}

	}

	return ddwFileSize;
}

int CQSEPSDF_CS::Work_Verify(const int32_t CurThreadNo, const int pointer)
{
	char szListName[300];
	char szlocal[300];
	char szremote[300];
	char szFile[300];
	char szMd5File[300];
	int  ret = 0;

	map<string, string> mapFile;
	string strres;
	sprintf(szListName, "%s.Ready", m_LstInf[CurThreadNo].m_CheckLstMName);

	FILE *fprLst = fopen(szListName, "rt");
	if (NULL == fprLst)
	{
		LOG("%s NULL Pointer\n", szListName);
		return -1;
	}

	//char* pBuf = new char[MAX_SEND_SIZE];
	while(fgets(szlocal, 300, fprLst))
	{
		if (GetrealStr(szlocal) <= 1)
			continue;
		char* p = strrchr(szlocal, '/');
		if(p != NULL)
			strcpy(szFile, p + 1);  //获取纯文件名
		else
			strcpy(szFile, szlocal);

		if(0){// Added on 20131129 video rank文件名调换
			char szRank_0[300];
			char szRank_1[300];

			strcpy(szRank_0, "Eps_VideoRank_0.dat");
			strcpy(szRank_1, "Eps_VideoRank_1.dat");

			if(strcmp(szRank_0, szFile) == 0)
			{
				strcpy(szFile, szRank_1);
			}
			else if(strcmp(szRank_1, szFile) == 0)
			{
				strcpy(szFile, szRank_0);
			}

		}
		
		//远程文件名
        //for(int i = 0; i < m_LstInf[CurThreadNo].m_RealGroupNum; i++) {
            if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupRoot != NULL)
                sprintf(szremote, "%s/%s", m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupRoot, szFile);
            else if(m_LstInf[CurThreadNo].m_GroupRoot != NULL)
                sprintf(szremote, "%s/%s", m_LstInf[CurThreadNo].m_GroupRoot, szFile);
            else
                sprintf(szremote, "%s", szFile);

            sprintf(szMd5File, "%s.md5", szlocal);
            if(access(szMd5File, 0) == 0) {
                mapFile.insert(pair<string, string>(string(szlocal), string(szremote)));
            }
        //}
	}
	//delete [] pBuf;
	fclose(fprLst);

	//for (int j = 0; j < m_LstInf[CurThreadNo].m_RealGroupNum; j ++)
	//{
		if (m_LstInf[CurThreadNo].m_GroupInf[pointer].m_SendDataType == DF_SEND_DATA_MOD)
		{
			return 0;
		} 
		
		for(int k = 0; k < m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum; k ++)
		{
			WorkClient wclient(string(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_Server[k].m_ServerIPStr), 
					m_LstInf[CurThreadNo].m_GroupInf[pointer].m_Server[k].m_ServerPort);
			
			for(map<string, string>::iterator itr = mapFile.begin(); itr != mapFile.end(); itr++)
			{
				ret = wclient.checkMD5(itr->first.c_str(), itr->second.c_str());
				if(ret)
				{
					wclient.close();
					LOG("[%s:%d]: '%s'check fail, ret:[%d]\n", m_LstInf[CurThreadNo].m_GroupInf[pointer].m_Server[k].m_ServerIPStr, 
						m_LstInf[CurThreadNo].m_GroupInf[pointer].m_Server[k].m_ServerPort,
						itr->second.c_str(), ret);
					return ret;
				}
				/*
				LOG("[%s:%d]: %s checked\n", m_LstInf[CurThreadNo].m_GroupInf[j].m_Server[k].m_ServerIPStr, 
						m_LstInf[CurThreadNo].m_GroupInf[j].m_Server[k].m_ServerPort,
						itr->second.c_str());
				 */
				
			}
			LOG("[%s:%d] Verify [%d] Files\n", 
					m_LstInf[CurThreadNo].m_GroupInf[pointer].m_Server[k].m_ServerIPStr, 
					m_LstInf[CurThreadNo].m_GroupInf[pointer].m_Server[k].m_ServerPort,
					mapFile.size());
			
		}

	//}

	
	return 0;
}

int CQSEPSDF_CS::Work_MoveMD5File(const int32_t CurThreadNo)
{
	char szListName[300];
	char szlocal[300];
	char szmd5[300];
	sprintf(szListName, "%s.Ready", m_LstInf[CurThreadNo].m_CheckLstMName);

	FILE *fprLst = fopen(szListName, "rt");
	if (NULL == fprLst)
	{
		LOG("%s NULL Pointer\n", szListName);
		return -1;
	}

	while(fgets(szlocal, 300, fprLst))
	{
		if (GetrealStr(szlocal) <= 1)
			continue;
		sprintf(szmd5, "%s.md5", szlocal);
		remove(szmd5);
	}
	fclose(fprLst);

	LOG("Remove List %s MD5 Files\n", m_LstInf[CurThreadNo].m_CheckLstMName);

	return 0;
}

int CQSEPSDF_CS::BuildMd5(const char* szFile, string& strmd5, char* pBuf)
{
	/*
	char szMdFile[300];
	sprintf(szMdFile, "%s.md5", szFile);
	if(access(szMdFile, 0) == 0)
	{
		return 0;
	}
	*/

	//fseek(fpr, 0, SEEK_END);
	//uint64_t ddwFileSize = ftell(fpr);
	MD5 md5;
	struct stat stbuf;
	stat(szFile, &stbuf);
	uint64_t ddwFileSize = (uint64_t)(stbuf.st_size);

	if(ddwFileSize > MAX_FILE_SIZE)
	{
		FILE* fpr = fopen(szFile, "rb");
		if(!fpr)
		{
			LOG("can not open %s\n", szFile);
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

	LOG("%s [%ld Bytes] md5:'%s'\n", szFile, ddwFileSize, strmd5.c_str());

	return 0;
}

int CQSEPSDF_CS::BuildDataToStr(_DFIELD_* pData, string& result)
{
	result = "";

	if(pData->m_OperateType == DF_NOT_OPR)
	{
		LOG("Operate Type error: [%d]\n", pData->m_OperateType);
		return -1;
	}

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);

	//Have List
	if(pData->m_ListNameLen > 0)
	{
		result.append((const char*)&pData->m_ListNameLen, 4);
		result.append(pData->m_Listname, pData->m_ListNameLen);
	}
	
	//Have File
	if(pData->m_FileNameLen > 0)
	{
		result.append((const char*)&pData->m_FileNameLen, 4);
		result.append(pData->m_Filename, pData->m_FileNameLen);
	}

	//Have Data
	if(pData->m_DataLength > 0)
	{
		result.append((const char*)&pData->m_DataLength, 4);
		result.append(pData->m_PData, pData->m_DataLength);
	}

	return 0;
}



int CQSEPSDF_CS::MainWork()
{
	int CurThreadNo =-1;

	EnterCriticalSection(&m_MainCri);
	CurThreadNo = m_ThreadNO;
	m_ThreadNO++;
	LeaveCriticalSection(&m_MainCri);

	char* RBuf = new char[1<<20];
	memset(RBuf, 0, 1<<20);

	char* pBuf = new char[MAX_SEND_SIZE];

	int k = 0, j = 0;

	char szlstName[300];
	sprintf(szlstName, "%s.Ready", m_LstInf[CurThreadNo].m_CheckLstMName);

	char datafile[300] = {0};  //合理的文件名需要注意，带有路径信息
	char szFile[300] = {0};

	RStreamBuf  rstream_;
	uint64_t ddwFileSize = 0;
	char *p = NULL;

#ifdef NLOG
	printf("Thread[%d] <--> %s\n", CurThreadNo, szlstName);
	LOG("Thread[%d] <--> %s\n", CurThreadNo, szlstName);

#endif
	///////////////////////////////////////
	//用于临时存储数据

    
    ////////////////////////////////////////////////////////////////
    //数据传输细化到Group,根据Group配置的不同，将数据传输到不同的远程目录中
    for(;1;) {
        _DFIELD_ TmpData;
        TmpData.m_FP = NULL;
        TmpData.m_DataSize = 0;
        TmpData.m_DataLength = 0;
        TmpData.m_PData = RBuf;
        TmpData.m_OPNO = CurThreadNo;
        TmpData.m_MemType = DF_NOT_NEED_MEM;
        TmpData.m_OperateType = DF_NOT_OPR;
       
        _LSTNO_ TmpLstNOINF;
        int iFileIdx = 0;
        int ret = 0;
        int iSendNum = 0;
        string strMd5;
        bool bverify = false;

        //   step 1: 监控指定的List

        // if *.lst.Ready exists, read the LstNO info, resume the break-point transmission
        if(access(szlstName, 0) == 0) {
             m_MainControl->GetLstNO(&TmpLstNOINF, CurThreadNo);
             m_LstInf[CurThreadNo].m_CurLstKey = TmpLstNOINF.m_LSTNO; //当前处理的List的key
             LOG("%s break point retransmission, had transmit File Num:[%d]\n", szlstName, TmpLstNOINF.m_FileNum);
        }
        else {
            if(rename(m_LstInf[CurThreadNo].m_CheckLstMName, szlstName)) {
                sleep(10);
                continue;
            }
            LOG("rename : %s -> %s\n", m_LstInf[CurThreadNo].m_CheckLstMName, szlstName);
            m_LstInf[CurThreadNo].m_CurLstKey = CurThreadNo;
            TmpLstNOINF.m_Time = time(NULL);
            TmpLstNOINF.m_FileNum = 0;
            TmpLstNOINF.m_AllSize = 0;
        }

        FILE* fprLst = fopen(szlstName, "rt");
        if(NULL == fprLst) {
            LOG("%s NULL Pointer\n", szlstName);
            continue;
        }
        if(feof(fprLst)) {
            LOG("List Empty: %s\n", szlstName);
            fclose(fprLst);
            continue;
        }
        if(fgets(datafile, 300, fprLst) == NULL) {
            LOG("List Empty: %s\n", szlstName);
            fclose(fprLst);
            continue;
        }
        else
            fseek(fprLst, 0, SEEK_SET);
        LOG("DFSystem Start List: %s\n", szlstName);
        for(int pointer = 0; pointer < m_LstInf[CurThreadNo].m_RealGroupNum; pointer++) {
            m_LstInf[CurThreadNo].m_CurFileFP = NULL;
            fprLst = fopen(szlstName, "rt");
            fseek(fprLst, 0, SEEK_SET);
            strMd5 = "";
            bverify = false;
            if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupLstName != NULL)
                strcpy(TmpData.m_Listname, m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupLstName);
            else
                strcpy(TmpData.m_Listname, m_LstInf[CurThreadNo].m_GroupLstName);
            TmpData.m_ListNameLen = strlen(TmpData.m_Listname);
            
            //向群组队列发送指令
            //   step 2: 打开List命令
            if(TmpLstNOINF.m_FileNum == 0)
                Work_StartList(CurThreadNo, pointer, &TmpData);
#ifdef DEBUG
            if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupLstName != NULL)
                cout<<"start list"<<m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupLstName<<endl;
            else
                cout<<"start list"<<m_LstInf[CurThreadNo].m_GroupLstName<<endl;
#endif
            
            iFileIdx = 0;
            iSendNum = 0;
            while(fgets(datafile, 300, fprLst)) {
                if(iFileIdx++ < TmpLstNOINF.m_FileNum) {
                    LOG("skip File:%s", datafile);
                    continue;
                }
                if(GetrealStr(datafile) <= 1)
                    continue;
                FILE* fprData = fopen(datafile, "rb");
                if(NULL == fprData) {
                    LOG("fuke: %s read error [%d]\n", datafile, errno);
                    continue;
                }

                p = strrchr(datafile, '/');
                if(NULL != p)
                    strcpy(szFile, p + 1);
                else
                    strcpy(szFile, datafile);

                if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupRoot != NULL)
                    sprintf(TmpData.m_Filename, "%s/%s", m_LstInf[CurThreadNo].m_GroupInf[pointer].m_GroupRoot, szFile);
                else
                    sprintf(TmpData.m_Filename, "%s/%s", m_LstInf[CurThreadNo].m_GroupRoot, szFile);
                TmpData.m_FileNameLen = strlen(TmpData.m_Filename);
                TmpData.m_FP = fprData;
                m_LstInf[CurThreadNo].m_CurFileFP = fprData;

                // step 3:打开文件

                Work_OpenFile(CurThreadNo, pointer, &TmpData);

                //step 4: 写入文件

                rewind(fprData);
                if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_SendDataType == DF_SEND_DATA_MOD) {
#ifdef DEBUG
                    cout<<"MOD"<<endl;
#endif
                    rstream_.pfr_ = fprData;
                    ddwFileSize = Work_ModData(CurThreadNo, pointer, &TmpData, &rstream_);
                    LOG("%s, MOD[%lu Bytes]\n", datafile, ddwFileSize);
                }
                else if(m_LstInf[CurThreadNo].m_GroupInf[pointer].m_SendDataType == DF_SEND_DATA_ALL) {
#ifdef DEBUG
                    cout<<"ALL"<<endl;
#endif
                    ddwFileSize = Work_ALLData(CurThreadNo, pointer, &TmpData, fprData);
                    LOG("%s, ALL[%lu Bytes]\n", datafile, ddwFileSize);
                    bverify = true;
                }

                fclose(fprData);
                iSendNum ++;

                // step 5:发送关闭文件的命令

                Work_CloseFile(CurThreadNo, pointer, &TmpData);
                sleep(1);

                if(bverify)
                    BuildMd5(datafile, strMd5, pBuf);
            }

            LOG("List %s Send [%d] Files\n", m_LstInf[CurThreadNo].m_CheckLstMName, iSendNum);
            fclose(fprLst);

            // step 6: verify Data

            if(bverify) {
                usleep(2000);
                for(j = 0 ; j < m_LstInf[CurThreadNo].m_GroupInf[pointer].m_RealServerNum; j++) {
                    while(m_pDataQue[CurThreadNo][pointer][j]->GetFreeNum() != m_MainControl->getQueSize())
                        usleep(2000);
                }

                sleep(1);
                ret = Work_Verify(CurThreadNo, pointer);
                if(ret) {
                    char szMsg[300];
                    snprintf(szMsg, sizeof(szMsg), "[%s Error]: %s Verity!",m_szLocalIp, szlstName);
                    SendSmsNotify(szMsg);
                    LOG("List %s Verity error, ret:[%d]!\n", szlstName, ret);
                    m_MainControl->ResetLstNO(CurThreadNo);
                    continue;
                }
            }
            
            //step 7: 发送关闭List的命令
            Work_CloseList(CurThreadNo, pointer, &TmpData);
            Work_MoveMD5File(CurThreadNo);
        }

        //step 8: 对List重命名，标记List处理完成

        char szTmpListName[300];
        sprintf(szTmpListName, "%s.Finish", m_LstInf[CurThreadNo].m_CheckLstMName);
        ret = rename(szlstName, szTmpListName);
        if(ret)
            LOG("rename error: %s -> %s, ret [%d], errno: %d\n",szlstName, szTmpListName, ret, errno);
        LOG("rename: %s -> %s\n",szlstName, szTmpListName);
    }

    delete[] RBuf;
    delete[] pBuf;
    delete p;
    return 0;
}

int CQSEPSDF_CS::Main_Command(int LNo, int GNo, int SNo)
{
   int CurLstNo = LNo;
   int CurGroupNO = GNo;
   int CurServerNO = SNo;

   //printf("%s: LstNo:%d,GrpNo:%d,SrvNo:%d,this:%p\n",__FUNCTION__, CurLstNo, CurGroupNO, CurServerNO, this);

   if(LNo > MAX_LST_NUM || GNo > MAX_GROUP_NUM || SNo > MAX_S_NUM_AGROUP)
   {
#ifdef NLOG
	   printf("(%s:%d): error\n", __FUNCTION__, __LINE__);
#endif
	   LOG("Parameter error\n");
   }

   /*EnterCriticalSection(&m_MainCri);
   //TODO 这部分有错，得按实际的数目计算
	CurLstNo = m_ThreadNO1/(MAXGROUPNUM*MAX_S_NUM_AGROUP);
	CurGroupNO =(m_ThreadNO1%(MAXGROUPNUM*MAX_S_NUM_AGROUP))/MAX_S_NUM_AGROUP;
	CurServerNO = (m_ThreadNO1%(MAXGROUPNUM*MAX_S_NUM_AGROUP))%MAX_S_NUM_AGROUP;
	m_ThreadNO1++;
   LeaveCriticalSection(&m_MainCri);
   */ 
   char *ModBuf = new char[1<<20];
   _DFIELD_* pData = new _DFIELD_; 
   pData->m_OperateType = DF_WRITE_FILE;
   pData->m_OPNO = 0;                          //LST的编号
   pData->m_PData = NULL;                      //数据存储的地址
   pData->m_FP = NULL;                         //文件句柄
   pData->m_Result = -1;                       //操作是否成功的标志
   pData->m_MemType = SUITE;
   int iQueNo = -1;

   for (;1;)
   {
	   iQueNo = m_pDataQue[CurLstNo][CurGroupNO][CurServerNO]->Pop();
	   if (iQueNo == -1)
	   {
		   Sleep(1);
		   continue;
	   }
	   
	   switch (m_MainControl->m_DataF[iQueNo].m_OperateType)
	   {
	      case DF_START_LST :
			  CreateALst(&m_MainControl->m_DataF[iQueNo], &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);
			  break;
		  case DF_END_LST:
			  EndALst(&m_MainControl->m_DataF[iQueNo], &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);
			  break;
		  case DF_OPEN_FILE :
			  OpenAFile(&m_MainControl->m_DataF[iQueNo], &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);
			  break;
		  case DF_CLOSE_FILE :
			  CloseAFile(&m_MainControl->m_DataF[iQueNo], &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);
			  break;
		  case DF_WRITE_FILE :
			  if(m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_SendDataType == DF_SEND_DATA_MOD)
			  {
				  int loop = 0;
				  *ModBuf = '\0';
				  
				  pData->m_DataSize = 0;
				  pData->m_NO = iQueNo;
				  pData->m_OPNO = m_MainControl->m_DataF[iQueNo].m_OPNO;

				  strcpy(pData->m_Filename, m_MainControl->m_DataF[iQueNo].m_Filename);
				  pData->m_FileNameLen = m_MainControl->m_DataF[iQueNo].m_FileNameLen;

				  strcpy(pData->m_Listname, m_MainControl->m_DataF[iQueNo].m_Listname);
				  pData->m_ListNameLen = m_MainControl->m_DataF[iQueNo].m_ListNameLen;

				  int iLen = m_MainControl->m_DataF[iQueNo].m_DataLength;//strlen(m_MainControl->m_DataF[iQueNo].m_PData);
				  memcpy(ModBuf, m_MainControl->m_DataF[iQueNo].m_PData, iLen);
				  //pData->m_DataSize += m_MainControl->m_DataF[pNo].m_DataSize - 1;
				  pData->m_PData = ModBuf + iLen;

				  m_MainControl->FreeAField(&m_MainControl->m_DataF[iQueNo]);


				  do
				  {
					  iQueNo = m_pDataQue[CurLstNo][CurGroupNO][CurServerNO]->Pop();
					  if(iQueNo == -1)
					  {
						  break;
					  }

					  if(m_MainControl->m_DataF[iQueNo].m_OperateType != DF_WRITE_FILE)
					  {
						  //m_pDataQue[CurLstNo][CurGroupNO][CurServerNO]->Push_W(pNO);

#ifdef DEBUG
						  LOG("operatetype:%d\n", m_MainControl->m_DataF[iQueNo].m_OperateType);
#endif
						  break;
					  }

					  iLen = m_MainControl->m_DataF[iQueNo].m_DataLength;//strlen(m_MainControl->m_DataF[iQueNo].m_PData);

					  memcpy(pData->m_PData, m_MainControl->m_DataF[iQueNo].m_PData, iLen);
					  pData->m_PData += iLen;

					  m_MainControl->FreeAField(&m_MainControl->m_DataF[iQueNo]);
					  loop++;

				  }while(m_MainControl->m_DataF[iQueNo].m_OperateType == DF_WRITE_FILE && loop < 666);

				  *(pData->m_PData) = '\0';
				  pData->m_DataSize = pData->m_PData - ModBuf;
				  pData->m_DataLength = pData->m_DataSize;
				  pData->m_PData = ModBuf;
				  //printf("pData:'%s',[%d]\n", pData->m_PData, pData->m_DataSize);

				  WriteADataToFile(pData, &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);

				  if(m_MainControl->m_DataF[iQueNo].m_OperateType == DF_CLOSE_FILE)
					  CloseAFile(&m_MainControl->m_DataF[iQueNo], &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);

			  }else{
				  WriteADataToFile(&m_MainControl->m_DataF[iQueNo], &m_LstInf[CurLstNo].m_GroupInf[CurGroupNO].m_Server[CurServerNO]);
			  }
			  break;
		  default:
			  //log
			  LOG("Operate Type error:[%d]\n", m_MainControl->m_DataF[iQueNo].m_OperateType);
			  break;
			  
	   }

   }

   delete[] ModBuf;
   delete pData;
   return 0;

}

void* CQSEPSDF_CS::Thread_MainWork(void* lpvd)
{
    CQSEPSDF_CS *p = (CQSEPSDF_CS*)lpvd;
	p->MainWork();
	return p;
}


void* CQSEPSDF_CS::Thread_Main_Command(void* lpvd)
{
	_LGS_* pLGS = (LstGrpSrv*)lpvd;

    CQSEPSDF_CS *p = (CQSEPSDF_CS*)(pLGS->lpvd);
#ifdef NLOG
	LOG("[%s:%d]LstNo:%d,GrpNo:%d,SrvNo:%d\n", p->m_LstInf[pLGS->LstNO].m_GroupInf[pLGS->GrpNO].m_Server[pLGS->SrvNO].m_ServerIPStr,
			p->m_LstInf[pLGS->LstNO].m_GroupInf[pLGS->GrpNO].m_Server[pLGS->SrvNO].m_ServerPort,
			pLGS->LstNO, pLGS->GrpNO, pLGS->SrvNO);

	printf("[%s:%d]LstNo:%d,GrpNo:%d,SrvNo:%d\n", p->m_LstInf[pLGS->LstNO].m_GroupInf[pLGS->GrpNO].m_Server[pLGS->SrvNO].m_ServerIPStr,
			p->m_LstInf[pLGS->LstNO].m_GroupInf[pLGS->GrpNO].m_Server[pLGS->SrvNO].m_ServerPort,
			pLGS->LstNO, pLGS->GrpNO, pLGS->SrvNO);
#endif

	p->Main_Command(pLGS->LstNO, pLGS->GrpNO, pLGS->SrvNO);
	return p;
	
}

int CQSEPSDF_CS::Start()
{	
	
	pthread_t dwId1;
	/*pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 99;
    pthread_attr_setschedparam(&attr, &param);*/
	
	int ret = 0 ;
	int  i(0),j(0),k(0);

	for ( i = 0; i < m_LstNum; i++ )
	{	
		if ((ret = pthread_create(&dwId1,NULL,&Thread_MainWork,(void *)this)) < 0 )
		{
			LOG("Thread[%d] start error\n", i);
			return -1;
		}
		pthread_detach(dwId1);
	}
	
	
	for ( i = 0; i < m_LstNum; i++ )
	{
		for ( j = 0; j < m_LstInf[i].m_RealGroupNum; j++ )
		{
			
			for ( k = 0; k < m_LstInf[i].m_GroupInf[j].m_RealServerNum; k++ )
			{

				m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerHand = new TCPClient(string(m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerIPStr), m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerPort, 10000);
				//para = new _LGS_(i,j,k,(CQSEPSDF_CS*)this);

				_LGS_ *para = new _LGS_(i, j, k, (CQSEPSDF_CS*)this);

				if ((ret = pthread_create(&dwId1,NULL,&Thread_Main_Command,(void *)para)) < 0 )
				{
					LOG("create thread[%d:%d:%d]:%s:%d\n", i,j,k,m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerIPStr,
							m_LstInf[i].m_GroupInf[j].m_Server[k].m_ServerPort);
					return -1;
				}
				pthread_detach(dwId1);

			}

		}
	}
	
	//pthread_attr_destroy(&attr);
	
	return 0;
}
