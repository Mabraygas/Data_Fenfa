/**
 * @file data_verify.cpp
 * @brief 数据校验
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-19
 */

#include <string>
#include <iostream>
#include <net/if.h>

#include <tinyxml.h>

#include "thread_update.h"
#include "DayLog.h"
#include "SmsNotify.h"


#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;

DayLog g_log;

ThreadUpdate::ThreadUpdate()
{
	m_GrpNum      = MAX_GRP_CNT;
	m_tid         = 0;
	m_iUpdateHour = 2;
	m_dwbrokerCnt = 2;
}

ThreadUpdate::~ThreadUpdate()
{
}

int ThreadUpdate::InitSys(const char* config, int iUpdateHour)
{
	g_log.Init(30, "../log/Base_Update");

	getLocalIP(m_szLocalIP);

	LOG("====================================VBase Update is running==============================\n");
	int Ret =LoadAllINF(config); 
	if (Ret)
	{
		LOG("Load Info error, return %d\n", Ret);
		return Ret; 
	}

	m_iUpdateHour = iUpdateHour;
	printf("update time: clock %d\n", iUpdateHour);
	LOG("update time: clock %d\n", iUpdateHour);

	/*
   strcpy(m_aBrokerIP[0], BROKER_IP_ONE);
   strcpy(m_aBrokerIP[1], BROKER_IP_TWO);
   */


   for(int i = 0; i < m_dwbrokerCnt; i ++)
   {
	   LOG("Broker[%d] %s\n", i, m_aBrokerIP[i]);
   }
	return 0;
}

/*
*/
int ThreadUpdate::LoadAllINF(const char* config)
{
   int i,j;
   for (i = 0; i < MAX_GRP_CNT; i++)
   {
	    m_aGrp[i].m_iKey         = 0;
		m_aGrp[i].m_ServerCount  = MAX_SERVER_CNT;
		*m_aGrp[i].m_LocalRoot   = 0;

	   for (j = 0; j < MAX_SERVER_CNT; j++)
	   {
		   m_aGrp[i].m_Server[j].m_ServerID    = j;
		   *m_aGrp[i].m_Server[j].m_RemoteRoot = 0;
	   }
   }
   //TODO 读取信息

   char xmlFile[200];	
   if(config == NULL)
   {
	   sprintf(xmlFile, "base.xml");
   }
   else
   {
	   strcpy(xmlFile, config);
   }
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

    FILE* fpin = fopen(BASE_INI, "rb");
	if(fpin == NULL)
	{
		printf("load default\n");

		strcpy(m_aBrokerIP[0], BROKER_IP_ONE);
		strcpy(m_aBrokerIP[1], BROKER_IP_TWO);

		m_dwbrokerCnt = 2;
		return 0;
	}

	char szTmp[30];
	i = 0;
	while(fgets(szTmp, 30, fpin)!=NULL)
	{
		GetrealStr(szTmp);
		strcpy(m_aBrokerIP[i], szTmp);
		//printf("%s\n", m_aBrokerIP[i]);
		i++;
	}
	m_dwbrokerCnt = i;
	fclose(fpin);
   return 0;
}

/*
*/
int ThreadUpdate::LoadXml(const char* xmlFile)
{
   if(xmlFile == NULL)
   {
       printf("xml name is null\n");
       return -1;
   }
   int i(0),j(0);
   TiXmlDocument doc;  
   if (!doc.LoadFile(xmlFile)) {  	
	   LOG("can not parse xml %s\n", xmlFile);
	   return -2;
   }

   TiXmlElement* rootElement = doc.RootElement();  //Soku元素  
   TiXmlAttribute* attributeOfSoku = rootElement->FirstAttribute();
	
   if(strcmp("GroupNumber", attributeOfSoku->Name()) == 0)
	   m_GrpNum = attributeOfSoku->IntValue();


   TiXmlElement* GroupElement = rootElement->FirstChildElement();//Group 
   for (i = 0; GroupElement != NULL; GroupElement = GroupElement->NextSiblingElement(), i++ ) 
   {
	   TiXmlAttribute* attributeOfGroup = GroupElement->FirstAttribute(); 
	   for (;attributeOfGroup != NULL; attributeOfGroup = attributeOfGroup->Next() ) 
	   {  
		   if(strcmp(attributeOfGroup->Name(), "LocalRoot") == 0)
		   {
			   strcpy(m_aGrp[i].m_LocalRoot, attributeOfGroup->Value());
			   int iLen = strlen(m_aGrp[i].m_LocalRoot);
			   if(m_aGrp[i].m_LocalRoot[iLen-1] == '/')
				   m_aGrp[i].m_LocalRoot[iLen-1] = '\0';
			   LOG("Group Local Root: %s\n", m_aGrp[i].m_LocalRoot);
		   }
		   else if(strcmp(attributeOfGroup->Name(), "key") == 0)
		   {
			   m_aGrp[i].m_iKey = attributeOfGroup->IntValue();
			   LOG("Group Key: %d\n", m_aGrp[i].m_iKey);
		   }
		   else if(strcmp(attributeOfGroup->Name(), "ServerCount") == 0)
		   {
			   m_aGrp[i].m_ServerCount = attributeOfGroup->IntValue();
			   LOG("Server Count: %d\n", m_aGrp[i].m_ServerCount);
		   }
	   }


	   TiXmlElement* ServerElement = GroupElement->FirstChildElement();//Server 
	   for (j = 0; ServerElement != NULL; ServerElement = ServerElement->NextSiblingElement(), j++ ) 
	   {
		   TiXmlAttribute* attributeOfServer = ServerElement->FirstAttribute(); 
		   for (;attributeOfServer != NULL; attributeOfServer = attributeOfServer->Next() ) 
		   {  
			   if(strcmp(attributeOfServer->Name(), "RemoteRoot") == 0)
			   {
				   strcpy(m_aGrp[i].m_Server[j].m_RemoteRoot, attributeOfServer->Value());
				   int iLen = strlen(m_aGrp[i].m_Server[j].m_RemoteRoot);
				   if(m_aGrp[i].m_Server[j].m_RemoteRoot[iLen-1] == '/')
					   m_aGrp[i].m_Server[j].m_RemoteRoot[iLen-1] = '\0';
				   LOG("Remote Root: %s\n", m_aGrp[i].m_Server[j].m_RemoteRoot);
			   }
			   else if(strcmp(attributeOfServer->Name(),"IP") == 0)
			   {
				   m_aGrp[i].m_Server[j].m_Client.setIP(string(attributeOfServer->Value()));
				   LOG("Remote IP: %s\n", m_aGrp[i].m_Server[j].m_Client.getIP().c_str());
			   }
			   else if(strcmp(attributeOfServer->Name(),"Port") == 0)
			   {
				   m_aGrp[i].m_Server[j].m_Client.setPort(attributeOfServer->IntValue());
				   LOG("Remote Port: %d\n", m_aGrp[i].m_Server[j].m_Client.getPort());
			   }
			   else if(strcmp(attributeOfServer->Name(),"ServerID") == 0)
			   {
				   m_aGrp[i].m_Server[j].m_ServerID = attributeOfServer->IntValue();

				   LOG("ServerID: %d\n", m_aGrp[i].m_Server[j].m_ServerID);
			   }

		   }
		   m_aGrp[i].m_Server[j].m_Client.Initialize();

	   }
	   m_aGrp[i].m_ServerCount = j;

   }

   m_GrpNum = i;
   return 0;
}

int ThreadUpdate::GetrealStr(char* source)
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

int ThreadUpdate::TouchFile(const char * file)
{
	FILE* fpw = fopen(file, "wb");
	if(!fpw)
	{
		LOG("Touch File '%s' error\n", file);
	}
	time_t tmNow = time(NULL);
    struct tm stNow = * localtime(&tmNow);

	fprintf(fpw, "Touch Myself, Date:%d%02d%02d-%2d%2d%2d\n", 
			stNow.tm_year + 1900, stNow.tm_mon + 1, stNow.tm_mday,
			stNow.tm_hour, stNow.tm_min, stNow.tm_sec);

	fclose(fpw);
	return 0;
}

int ThreadUpdate::getLocalIP(char* outip)
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

void* ThreadUpdate::CallBack(void* lpvd)
{
    ThreadUpdate *p = (ThreadUpdate*)lpvd;

	p->run();
	return p;
}

void  ThreadUpdate::start()
{
	pthread_create(&m_tid, NULL, CallBack, (void *)this);
}

void  ThreadUpdate::join()
{
	if(m_tid)
	{
		pthread_join(m_tid, NULL);
		m_tid = 0;
	}
}


int ThreadUpdate::run()
{
	int  i  = 0, j = 0, ret = 0;
	//char szlocal[300] = {0};
	char szremote[300] = {0};
	char szservice[300] = {0};
	//char szMsg[300]   = {0};

	char szDate[300];
	int  idLen = 0;
	time_t tmNow;
	struct tm  stNow;

	char szLocalAllUpdate[300];
	char szLocalUpdateService[300];
	char szLocalTmp[300];
	char szExportEndOnN[4][300] = {{0}};

	char szExportUpdated[300];
	char szDelayName[4][300] = {{0}};


	//只考虑一个Group的情况
	sprintf(szLocalAllUpdate, "%s/%s", m_aGrp[0].m_LocalRoot, ALL_UPDATE_SYMBOLE);

	sprintf(szLocalUpdateService, "%s/%s", m_aGrp[0].m_LocalRoot, DONOT_MERGE_PLUS1M);

	for(i = 0; i < m_dwbrokerCnt; i++)
	{
		sprintf(szExportEndOnN[i], "%s/%s%s", m_aGrp[0].m_LocalRoot, EXPORT_END_ON_N, m_aBrokerIP[i]);
		LOG("[%d]%s\n", i, szExportEndOnN[i]);
		sprintf(szDelayName[i], "%s.Delay", szExportEndOnN[i]);
	}



	for (;1;)
	{

		tmNow = time(NULL);
		stNow = * localtime(&tmNow);

		// if not exists, or not the time, then continue
		if ((stNow.tm_hour != m_iUpdateHour) && access(szLocalAllUpdate, 0) < 0)
		{
			//printf("sleep:%s\n", szLocalAllUpdate);
			sleep(5);
			continue;
		} 

		for(i = 0; i < 2; i++)
		{
			if(rename(szExportEndOnN[i], szDelayName[i]) == 0)
				LOG("'%s' -> '%s'\n", szExportEndOnN[i], szDelayName[i]);
		}

		sprintf(szDate, "%d%02d%02d-%2d%2d%2d", stNow.tm_year + 1900, stNow.tm_mon + 1, stNow.tm_mday,
				stNow.tm_hour, stNow.tm_min, stNow.tm_sec);
		idLen = strlen(szDate);

		//printf("Start Update! Time:%s\n", szDate);

		/*-------------------------------------
		 * AllUpdate and UpdateService
		 *-------------------------------------
		 */
		for(i = 0; i < m_GrpNum; i++)
		{
			
			for (j = 0; j < m_aGrp[i].m_ServerCount; j++)
			{
				//远程文件名
				if(m_aGrp[i].m_Server[j].m_RemoteRoot == NULL)
				{
					sprintf(szremote, "%s", ALL_UPDATE_SYMBOLE);
					sprintf(szservice, "%s", DONOT_MERGE_PLUS1M);
				}
				else
				{
					sprintf(szremote, "%s/%s", m_aGrp[i].m_Server[j].m_RemoteRoot, ALL_UPDATE_SYMBOLE);
					sprintf(szservice, "%s/%s", m_aGrp[i].m_Server[j].m_RemoteRoot, DONOT_MERGE_PLUS1M);
				}

				//AllUpdate
				do
				{
					ret = m_aGrp[i].m_Server[j].m_Client.createOneFile(szremote, szDate, idLen);
					if(ret)
					{
						LOG("create %s error, ret[%d]", szremote, ret);
						usleep(2000);
					}
				}while(ret < 0);

				//UpdateService
				do{

					ret = m_aGrp[i].m_Server[j].m_Client.createOneFile(szservice, szDate, idLen);
					if(ret)
					{
						LOG("create %s error, ret[%d]", szservice, ret);
						usleep(2000);
					}
				}while(ret < 0);

				LOG("Update [%s:%d]:'%s' and '%s'\n", 
						m_aGrp[i].m_Server[j].m_Client.getIP().c_str(), 
						m_aGrp[i].m_Server[j].m_Client.getPort(),
						szremote, szservice);
				m_aGrp[i].m_Server[j].m_Client.close();
			}//end of for j

			LOG("Group %d have updated!\n", m_aGrp[i].m_iKey);

		}//end of for i

		//rename local AllUpdateSymbol
		if (access(szLocalAllUpdate, 0) == 0)
		{

			tmNow = time(NULL);
			stNow = * localtime(&tmNow);

			sprintf(szLocalTmp, "%s.%d%02d%02d-%02d%02d%02d", szLocalAllUpdate,
					stNow.tm_year + 1900, stNow.tm_mon + 1, stNow.tm_mday,
					stNow.tm_hour, stNow.tm_min, stNow.tm_sec);
			ret = rename(szLocalAllUpdate, szLocalTmp);
			if(ret)
			{
				LOG("Update rename %s -> %s error!, ret: [%d]\n", szLocalAllUpdate, szLocalTmp, ret);
			}

		}

		sleep(120);  //sleep 120seconds *one hour

		/*-------------------------------------
		 * Remove UpdateServiceSymbol
		 *-------------------------------------
		 */
		//TODO

		//Wait for ExportEndOnN 0 and 1

		tmNow = time(NULL);
		for(i = 0; i < m_dwbrokerCnt; i++)
		{
			do
			{
				if(time(NULL) - tmNow >= 9000)  //another 2.5 hours
				{
					//TouchFile(szExportEndOnN[i]);

					char szMsg[300];
					snprintf(szMsg, sizeof(szMsg), "[%s] %s delay!", m_szLocalIP, szExportEndOnN[i]);
					SendSmsNotify(szMsg);

					//LOG("[Warn] Touch '%s' Myself\n", szExportEndOnN[i]);
					LOG("[Warn]'%s' Delay\n", szExportEndOnN[i]);
					tmNow = time(NULL);

				}

				ret = access(szExportEndOnN[i], 0);
				if(ret < 0)
				{
					sleep(1);
				}
			}while(ret < 0);
			LOG("%s appeared!\n", szExportEndOnN[i]);
		}
			

		for(i = 0; i < m_GrpNum; i++)
		{
			
			for (j = 0; j < m_aGrp[i].m_ServerCount; j++)
			{
				//远程文件名
				if(m_aGrp[i].m_Server[j].m_RemoteRoot == NULL)
				{
					sprintf(szservice, "%s", DONOT_MERGE_PLUS1M);
				}
				else
				{
					sprintf(szservice, "%s/%s", m_aGrp[i].m_Server[j].m_RemoteRoot, DONOT_MERGE_PLUS1M);
				}

				//UpdateService

				ret = m_aGrp[i].m_Server[j].m_Client.removeOneFile(szservice);
				if(ret)
				{
					LOG("remove %s error, ret[%d]", szservice, ret);

					char szMsg[300];
					snprintf(szMsg, sizeof(szMsg), "[Remove Error, ret %d],server[%s]", 
							ret, m_aGrp[i].m_Server[j].m_Client.getIP().c_str());
					SendSmsNotify(szMsg);

					usleep(2000);
				}

				LOG("Remove [%s:%d]:'%s'\n", 
						m_aGrp[i].m_Server[j].m_Client.getIP().c_str(), 
						m_aGrp[i].m_Server[j].m_Client.getPort(),
						szservice);
				m_aGrp[i].m_Server[j].m_Client.close();
			}//end of for j

		}//end of for i

		//TODO

		tmNow = time(NULL);
		stNow = * localtime(&tmNow);
		
		for(i = 0; i < m_dwbrokerCnt; i++)
		{
		
			if(access(szExportEndOnN[i], 0) == 0)
			{
				sprintf(szExportUpdated, "%s.%d%02d%02d.updated", 
						szExportEndOnN[i],
						stNow.tm_year + 1900, 
						stNow.tm_mon + 1, 
						stNow.tm_mday);

				ret = rename(szExportEndOnN[i], szExportUpdated);

				if(ret < 0)
				{
					char szMsg[300];
					snprintf(szMsg, sizeof(szMsg), "[%s Rename Error, ret %d]!", szExportEndOnN[i], ret);
					SendSmsNotify(szMsg);
				}

				LOG("Rename '%s' -> '%s', ret[%d]\n", szExportEndOnN[i], szExportUpdated, ret);
			}
		}

		sleep(3605);
	
		/*-----------------------------------
		 * End Remove UpdateServiceSymbol
		 *-----------------------------------
		 */


	}//end for (;1;)

	return 0;
}

int main(int argc, char** argv)
{
	int iTmUpdate = 2;
	if(argc > 1)
	{
		if(strcmp("-h", argv[1]) == 0)
		{
			printf("%s [Update Hour]\n", argv[0]);
			return -1;
		}
		else
		{
			iTmUpdate = atoi(argv[1]);
		}
	}
	ThreadUpdate tu;
	tu.InitSys(NULL, iTmUpdate);
	tu.start();

	tu.join();
	return 0;
}
