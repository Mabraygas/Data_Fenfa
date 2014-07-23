/**
 * @file SymbolMonitor.cpp
 * @brief 流程控制程序
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-12
 */


#include <tinyxml.h>

#include "symbol_monitor.h"
#include "DayLog.h"

#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

extern DayLog g_log;

using namespace std;

SymbolMonitor::SymbolMonitor()
{
	m_tid         = 0;
	m_apGroup     = NULL;
	m_apBases     = NULL;
	m_iCntBases   = MAX_BASES_NUM;
	m_iCntGroup   = MAX_GROUPS_NUM;

}

SymbolMonitor::~SymbolMonitor()
{
	delete [] m_apBases;
	for(int i = 0; i < m_iCntGroup; i++)
	{
		delete [] m_apGroup[i].m_apIS;
		delete [] m_apGroup[i].m_apBroker;
	}
	delete [] m_apGroup;
}

int SymbolMonitor::loadConfig(const char* xmlFile)
{
   int idxBase(0);
   char szTmp[300];
   int  iLen = 0;

   printf("%s\n", xmlFile);
   TiXmlDocument doc;  
   if (!doc.LoadFile(xmlFile)) 
   {  	
	   LOG("can not parse xml %s\n", xmlFile);
	   return -2;
   }

   TiXmlElement* rootElement = doc.RootElement();  //Config元素  
   TiXmlAttribute* attributeOfRoot = rootElement->FirstAttribute(); 
   m_iCntGroup = attributeOfRoot->IntValue();

   m_apGroup = new GroupInfo[m_iCntGroup];

   TiXmlElement* childElement = rootElement->FirstChildElement();  //Base or Group 
   for ( ; childElement != NULL; childElement = childElement->NextSiblingElement()) 
   { 
	   TiXmlAttribute* attributeOfChild = childElement->FirstAttribute(); 

	   if(strcmp(attributeOfChild->Name(), "BaseRoot") == 0)
	   {
			   strcpy(szTmp, attributeOfChild->Value());

			   iLen = strlen(szTmp);
			   if(szTmp[iLen-1] == '/')
				   szTmp[iLen-1] = '\0';

			   m_strBaseRoot = string(szTmp);

			   attributeOfChild = attributeOfChild->Next();
			   for (;attributeOfChild != NULL; attributeOfChild = attributeOfChild->Next() ) 
			   {
				   if(strcmp(attributeOfChild->Name(), "BaseCount") == 0)
				   {
					   m_iCntBases = attributeOfChild->IntValue();
					   m_apBases   = new WorkClient[m_iCntBases];

				   }

			   }

			   TiXmlElement* nodeElement = childElement->FirstChildElement();//node 
			   for ( ; nodeElement != NULL; nodeElement = nodeElement->NextSiblingElement()) 
			   {
				   TiXmlAttribute* attributeOfNode = nodeElement->FirstAttribute(); 
				   for (; attributeOfNode != NULL; attributeOfNode = attributeOfNode->Next()) 
				   {  
					   if(strcmp(attributeOfNode->Name(),"IP") == 0)
					   {
						   m_apBases[idxBase].setIP(string(attributeOfNode->Value()));
					   }
					   else if(strcmp(attributeOfNode->Name(),"Port") == 0)
					   {
						   m_apBases[idxBase].setPort(attributeOfNode->IntValue());
					   }
				   }
				   m_apBases[idxBase].Initialize();
				   idxBase++;

			   }


	   }
	   else if(strcmp(attributeOfChild->Name(), "GroupID") == 0)
	   {
		   int idxgrp = attributeOfChild->IntValue();
		   m_apGroup[idxgrp].m_iGroupID = idxgrp;

		   attributeOfChild = attributeOfChild->Next();
		   for (;attributeOfChild != NULL; attributeOfChild = attributeOfChild->Next()) 
		   {
			   if(strcmp(attributeOfChild->Name(), "ListName") == 0)
			   {
				   m_apGroup[idxgrp].m_strListName = string(attributeOfChild->Value());
			   }

		   }

		   //IS and Broker
		   TiXmlElement* isbrokerElement = childElement->FirstChildElement();//node 
		   for ( ; isbrokerElement != NULL; isbrokerElement = isbrokerElement->NextSiblingElement()) 
		   {
			   TiXmlAttribute* attributeOfIsBroker = isbrokerElement->FirstAttribute(); 
			   if(strcmp(attributeOfIsBroker->Name(),"ISRoot") == 0)
			   {
				   strcpy(szTmp, attributeOfIsBroker->Value());
				   iLen = strlen(szTmp);
				   if(szTmp[iLen-1] == '/')
					   szTmp[iLen-1] = '\0';

				   m_apGroup[idxgrp].m_strISRoot= string(szTmp);

				   attributeOfIsBroker = attributeOfIsBroker->Next();
				   for (; attributeOfIsBroker != NULL; attributeOfIsBroker = attributeOfIsBroker->Next()) 
				   {  
					   if(strcmp(attributeOfIsBroker->Name(),"ISCount") == 0)
					   {
						   m_apGroup[idxgrp].m_iCntIS = attributeOfIsBroker->IntValue();
						   m_apGroup[idxgrp].m_apIS = new WorkClient[m_apGroup[idxgrp].m_iCntIS];
					   }
				   }


				   TiXmlElement* isElement = isbrokerElement->FirstChildElement();
				   int idxis = 0;
				   for ( ; isElement != NULL && idxis < m_apGroup[idxgrp].m_iCntIS; isElement = isElement->NextSiblingElement()) 
				   {
					   TiXmlAttribute* attributeOfIs = isElement->FirstAttribute(); 
					   for (; attributeOfIs != NULL; attributeOfIs = attributeOfIs->Next()) 
					   { 
						   if(strcmp(attributeOfIs->Name(),"IP") == 0)
						   {
							   m_apGroup[idxgrp].m_apIS[idxis].setIP(string(attributeOfIs->Value()));
						   }
						   else if(strcmp(attributeOfIs->Name(),"Port") == 0)
						   {
							   m_apGroup[idxgrp].m_apIS[idxis].setPort(attributeOfIs->IntValue());
						   }

					   }
					   m_apGroup[idxgrp].m_apIS[idxis].Initialize();
					   idxis++;

				   }





			   }
			   else if(strcmp(attributeOfIsBroker->Name(),"BrokerRoot") == 0)
			   {
				   strcpy(szTmp, attributeOfIsBroker->Value());
				   iLen = strlen(szTmp);
				   if(szTmp[iLen-1] == '/')
					   szTmp[iLen-1] = '\0';

				   m_apGroup[idxgrp].m_strBrokerRoot= string(szTmp);

				   attributeOfIsBroker = attributeOfIsBroker->Next();
				   for (; attributeOfIsBroker != NULL; attributeOfIsBroker = attributeOfIsBroker->Next()) 
				   {  
					   if(strcmp(attributeOfIsBroker->Name(),"BrokerCount") == 0)
					   {
						   m_apGroup[idxgrp].m_iCntBroker = attributeOfIsBroker->IntValue();
						   m_apGroup[idxgrp].m_apBroker = new WorkClient[m_apGroup[idxgrp].m_iCntBroker];
					   }
				   }


				   TiXmlElement* brokerElement = isbrokerElement->FirstChildElement();
				   int idxbroker = 0;
				   for ( ; brokerElement != NULL && idxbroker < m_apGroup[idxgrp].m_iCntBroker; brokerElement = brokerElement->NextSiblingElement()) 
				   {
					   TiXmlAttribute* attributeOfBroker = brokerElement->FirstAttribute(); 
					   for (; attributeOfBroker != NULL; attributeOfBroker = attributeOfBroker->Next()) 
					   { 
						   if(strcmp(attributeOfBroker->Name(),"IP") == 0)
						   {
							   m_apGroup[idxgrp].m_apBroker[idxbroker].setIP(string(attributeOfBroker->Value()));
						   }
						   else if(strcmp(attributeOfBroker->Name(),"Port") == 0)
						   {
							   m_apGroup[idxgrp].m_apBroker[idxbroker].setPort(attributeOfBroker->IntValue());
						   }

					   }
					   m_apGroup[idxgrp].m_apBroker[idxbroker].Initialize();
					   idxbroker++;

				   }


			   }

		   }


	   }

   }


   printf("end of xml\n");
   return 0;

}

int SymbolMonitor::Init(const char* root, const char* configFile)
{
	char szRoot[300];
	char szConfig[300];
	int  iLen = 0;
	int  ret  = 0;
	if(root == NULL)
	{
		getcwd(szRoot, 300);
	}
	else
	{
		strcpy(szRoot, root);
	}

	
	iLen = strlen(szRoot);
	if(szRoot[iLen - 1] == '/')
		szRoot[iLen - 1] = 0;

	m_strRoot = string(szRoot);
	//TODO read configuration
	if(configFile == NULL)
	{
		sprintf(szConfig, "symbolconfig.xml");
	}
	else
	{
		strcpy(szConfig, configFile);
	}

	g_log.Init(30, "../log/SymbolMonitor");
	LOG("Symbol Monitor is running\n");

	ret = loadConfig(szConfig);
	if(ret)
	{
		LOG("loadConfig error, ret:[%d]\n", ret);
		return ret;
	}

	int i(0), j(0);

	for( i = 0; i < m_iCntBases; i ++)
	{
		LOG("Base[%d]%s:%d\n", i, m_apBases[i].getIP().c_str(), m_apBases[i].getPort());
	}

	for( i = 0; i < m_iCntGroup; i ++)
	{
		LOG("Group[%d]id:%d\tiCntIS:%d\tiCntBroker:%d\tList:%s\tISRoot:%s\tBrokerRoot:%s\n", 
				i, m_apGroup[i].m_iGroupID, m_apGroup[i].m_iCntIS,
				m_apGroup[i].m_iCntBroker, m_apGroup[i].m_strListName.c_str(), 
				m_apGroup[i].m_strISRoot.c_str(), m_apGroup[i].m_strBrokerRoot.c_str());
		for( j = 0; j < m_apGroup[i].m_iCntIS; j++)
		{

			LOG("IS[%d]%s:%d\n", j, m_apGroup[i].m_apIS[j].getIP().c_str(), m_apGroup[i].m_apIS[j].getPort());
		}

		for( j = 0; j < m_apGroup[i].m_iCntBroker; j++)
		{

			LOG("Broker[%d]%s:%d\n", j, m_apGroup[i].m_apBroker[j].getIP().c_str(), m_apGroup[i].m_apBroker[j].getPort());
		}

	}

	printf("Init end\n");

	//Local Symbol
	m_strBrokerStart   = m_strRoot     + "/" + BROKER_START_SYMBOLE; 

	//Remote Base
	m_strAllUpdate     = m_strBaseRoot + "/" + ALL_UPDATE_SYMBOLE;
	m_strUpdateService = m_strBaseRoot + "/" + DONOT_MERGE_PLUS1M;

	//Remote Broker
	//m_strTransmitEnd   = m_strBaseRoot + "/" + TRANSMIT_END_SYMBOLE;

	printf("broker:%s\n", m_strBrokerStart.c_str());
	printf("allupdate:%s\n", m_strAllUpdate.c_str());
	printf("updateservice:%s\n", m_strUpdateService.c_str());
	//printf("transmit:%s\n", m_strTransmitEnd.c_str());

	return 0;
}

void* SymbolMonitor::Thread_CallBack(void* lpvd)
{
    SymbolMonitor *p = (SymbolMonitor*)lpvd;

	p->run();
	return p;
}

void  SymbolMonitor::start()
{
	pthread_create(&m_tid, NULL, Thread_CallBack, (void *)this);
}

void  SymbolMonitor::join()
{
	if(m_tid)
	{
		pthread_join(m_tid, NULL);
		m_tid = 0;
	}
}

int SymbolMonitor::run()
{
	int i(0), j(0);
	int ret(-1);
	char szTmpStart[300];
	sprintf(szTmpStart, "%s.smtmp", m_strBrokerStart.c_str());
	
	for(;1;)
	{
        
		//step 1: check broker start symbol in local
	    if(access(m_strBrokerStart.c_str(), 0))    //not exist
		{
			sleep(1);
		    continue;
		}
		LOG("Broker Start Symbol exists\n");

		//stpe 2: rename BrokerStart symbol
		ret = rename(m_strBrokerStart.c_str(), szTmpStart);
		if(ret)
		{
			printf("rename error: %s -> %s\n", m_strBrokerStart.c_str(), szTmpStart);
			continue;
		}
		LOG("Rename %s -> %s\n", m_strBrokerStart.c_str(), szTmpStart);

		//step 3: allupdate and updateservice in Bases
		for(i = 0; i < m_iCntBases; i++)
		{
			do
			{
			    ret = m_apBases[i].createOneFile(m_strAllUpdate.c_str(), NULL, 0);
			    if(ret)
				     sleep(1);
			}while(ret);

			do
			{
			    ret = m_apBases[i].createOneFile(m_strUpdateService.c_str(), NULL, 0);
			    if(ret)
				     sleep(1);
			}while(ret);
			LOG("Base[%d] has created AllUpdateSymbol and UpdateServiceSymbol\n", i);

		}

		//sleep a while await the Bases update
		//sleep(300);
        
        //step 4: check send data finish in IS and Brokers of each group
		for(i = 0; i < m_iCntGroup; i++)
		{
			string strISRemoteList        = m_apGroup[i].m_strISRoot     + "/" + m_apGroup[i].m_strListName;
			string strTmpISRemoteList     = m_apGroup[i].m_strISRoot     + "/" + m_apGroup[i].m_strListName + ".smtmp";

			string strBrokerRemoteList    = m_apGroup[i].m_strBrokerRoot + "/" + m_apGroup[i].m_strListName;
			string strTmpBrokerRemoteList = m_apGroup[i].m_strBrokerRoot + "/" + m_apGroup[i].m_strListName + ".smtmp";


			//step 4.1: check List in IS and rename it
			for(j = 0; j < m_apGroup[i].m_iCntIS; j++)
			{
				do
				{
					ret = m_apGroup[i].m_apIS[j].renameFile(strISRemoteList.c_str(), strTmpISRemoteList.c_str());
					if(ret)
					{
						sleep(1);
					}
				}while(ret);

			}
			LOG("Group[%d], IS had recieved data\n", i);

			//step 4.2: check List in Broker and rename it
			for(j = 0; j < m_apGroup[i].m_iCntBroker; j++)
			{
				do
				{
					ret = m_apGroup[i].m_apBroker[j].renameFile(strBrokerRemoteList.c_str(), strTmpBrokerRemoteList.c_str());
					if(ret)
					{
						sleep(1);
					}
				}while(ret);

			}
			LOG("Group[%d], Broker had recieved data\n", i);
		}

		//step 5: create transmit end symbol in Broker 
		for(i = 0; i < m_iCntGroup; i++)
		{
			string strTransmitEnd = m_apGroup[i].m_strBrokerRoot + "/" + TRANSMIT_END_SYMBOLE;

			for(j = 0; j < m_apGroup[i].m_iCntBroker; j++)
			{
				do
				{
					ret = m_apGroup[i].m_apBroker[j].createOneFile(strTransmitEnd.c_str(), NULL, 0);
					if(ret)
					{
						sleep(1);
					}
				}while(ret);

			}
			LOG("Group[%d], Broker had created Transmit End Symbol\n", i);
		}

		//step 6: remove UpdateServiceSymbol.dat
        for(i = 0; i < m_iCntBases; i++)
        {
			do
			{
				ret = m_apBases[i].removeOneFile(m_strUpdateService.c_str());
				if(ret)
				{
					sleep(1);
				}
			}while(ret);
        }
		LOG("remove Bases' UpdateService Symbol\n");

		
		//next All Update Progress
		sleep(100);

	}//end for(;1;)

	return 0;
}



