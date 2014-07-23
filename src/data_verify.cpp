/**
 * @file data_verify.cpp
 * @brief 数据校验
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-19
 */

#include <string>
#include <iostream>

#include <tinyxml.h>

#include "data_verify.h"
#include "DayLog.h"
#include "SmsNotify.h"
#include "QSEPSDF_MainControl.h"

#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;

extern int MAX_LST_NUM, MAX_GROUP_NUM, MAX_S_NUM_AGROUP;

extern DayLog g_log;

DataVerify::DataVerify()
{
	m_LstNum = MAX_LST_NUM;
	m_tid    = 0;
    m_LstInf = new vList[MAX_LST_NUM];
}

DataVerify::~DataVerify()
{
    delete[] m_LstInf;
}

int DataVerify::InitSys(const char* config)
{
	g_log.Init(30, "../log/data_verify");

	int Ret =LoadAllINF(config); 
	if (Ret)
	{
		LOG("Load Info error, return %d\n", Ret);
		return Ret; 
	}

	LOG("Data verify is running\n");

	return 0;
}

/*
*/
int DataVerify::LoadAllINF(const char* config)
{
   int i,j;
   for (i=0; i<MAX_LST_NUM; i++)
   {
	    m_LstInf[i].m_RealGroupNum = 0;
		m_LstInf[i].m_CheckLstMName[0] = 0;
		*m_LstInf[i].m_GroupRoot = 0;
		*m_LstInf[i].m_GroupLstName = 0;

	   for (j=0; j<MAX_GROUP_NUM; j++)
	   {
		   m_LstInf[i].m_GroupInf[j].m_GroupNo = j;
		   m_LstInf[i].m_GroupInf[j].m_RealServerNum = 0;
		   m_LstInf[i].m_GroupInf[j].m_SendDataType = 256;
	   }
   }
   //TODO 读取信息

   char xmlFile[200];	
   if(config == NULL)
   {
	   sprintf(xmlFile, "soku.xml");
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
   return 0;
}

/*
*/
int DataVerify::LoadXml(const char* xmlFile)
{
   if(xmlFile == NULL)
   {
       printf("xml name is null\n");
       return -1;
   }
   int i(0),j(0),k(0);
   TiXmlDocument doc;  
   if (!doc.LoadFile(xmlFile)) {  	
	   LOG("can not parse xml %s\n", xmlFile);
	   return -2;
   }

   TiXmlElement* rootElement = doc.RootElement();  //Soku元素  
   TiXmlAttribute* attributeOfSoku = rootElement->FirstAttribute();
	
   if(strcmp("ListNumber", attributeOfSoku->Name()) == 0)
	   m_LstNum = attributeOfSoku->IntValue();


   TiXmlElement* ListElement = rootElement->FirstChildElement();  //List  

   for (i = 0; ListElement != NULL; ListElement = ListElement->NextSiblingElement(),i++ ) 
   {  
	   TiXmlAttribute* attributeOfList = ListElement->FirstAttribute(); 
	   for (;attributeOfList != NULL; attributeOfList = attributeOfList->Next() ) 
	   {  
		   //cout << attributeOfList->Name() << " : " << attributeOfList->Value() << std::endl;  
		   if(strcmp(attributeOfList->Name(), "name") == 0)
		   {
			   strcpy(m_LstInf[i].m_CheckLstMName, attributeOfList->Value());
			   //cout<<"name:"<<m_LstInf[i].m_CheckLstMName<<endl;
		   }else if(strcmp(attributeOfList->Name(), "GroupRoot") == 0)
		   {
			   strcpy(m_LstInf[i].m_GroupRoot, attributeOfList->Value());
			   int iLen = strlen(m_LstInf[i].m_GroupRoot);
			   if(m_LstInf[i].m_GroupRoot[iLen-1] == '/')
				   m_LstInf[i].m_GroupRoot[iLen-1] = '\0';
			   //cout<<"GroupFileName:"<<m_LstInf[i].m_GroupRoot<<endl;
		   }else if(strcmp(attributeOfList->Name(), "GroupListName") == 0)
		   {
			   strcpy(m_LstInf[i].m_GroupLstName, attributeOfList->Value());
			   //cout<<"GroupListName:"<<m_LstInf[i].m_GroupLstName<<endl;
		   }else if(strcmp(attributeOfList->Name(), "GroupNumber") == 0)
		   {
			   m_LstInf[i].m_RealGroupNum = attributeOfList->IntValue();
			   //cout<<"GroupNumber:"<<m_LstInf[i].m_RealGroupNum<<endl;
		   }
	   }


	   TiXmlElement* GroupElement = ListElement->FirstChildElement();//Group 
	   for (j = 0; GroupElement != NULL; GroupElement = GroupElement->NextSiblingElement(), j++ ) 
	   {
		   TiXmlAttribute* attributeOfGroup = GroupElement->FirstAttribute(); 
		   for (;attributeOfGroup != NULL; attributeOfGroup = attributeOfGroup->Next() ) 
		   {  
			   //cout << attributeOfGroup->Name() << " : " << attributeOfGroup->Value() << std::endl;  
			   if(strcmp(attributeOfGroup->Name(), "GroupNo") == 0)
			   {
				   m_LstInf[i].m_GroupInf[j].m_GroupNo = attributeOfGroup->IntValue();
				   //cout<<"GroupNo:"<<m_LstInf[i].m_GroupInf[j].m_GroupNo<<endl;
			   }else if(strcmp(attributeOfGroup->Name(), "SendDataType") == 0)
			   {
				   m_LstInf[i].m_GroupInf[j].m_SendDataType = attributeOfGroup->IntValue();
				   //cout<<"SendDataType:"<<m_LstInf[i].m_GroupInf[j].m_SendDataType<<endl;
			   }else if(strcmp(attributeOfGroup->Name(), "ServerNumber") == 0)
			   {
				   m_LstInf[i].m_GroupInf[j].m_RealServerNum = attributeOfGroup->IntValue();
				   //cout<<"ServerNumber:"<<m_LstInf[i].m_GroupInf[j].m_RealServerNum<<endl;
			   }
		   }


		   TiXmlElement* ServerElement = GroupElement->FirstChildElement();//Server 
		   for (k = 0; ServerElement != NULL; ServerElement = ServerElement->NextSiblingElement(), k++ ) 
		   {
			   TiXmlAttribute* attributeOfServer = ServerElement->FirstAttribute(); 
			   for (;attributeOfServer != NULL; attributeOfServer = attributeOfServer->Next() ) 
			   {  
				   if(strcmp(attributeOfServer->Name(),"IP") == 0)
				   {
					   m_LstInf[i].m_GroupInf[j].m_Server[k].setIP(string(attributeOfServer->Value()));
				   }else if(strcmp(attributeOfServer->Name(),"Port") == 0)
				   {
					   m_LstInf[i].m_GroupInf[j].m_Server[k].setPort(attributeOfServer->IntValue());
				   }
			   }
			   m_LstInf[i].m_GroupInf[j].m_Server[k].Initialize();

		   }
		   m_LstInf[i].m_GroupInf[j].m_RealServerNum = k;

	   }
	   m_LstInf[i].m_RealGroupNum  = j;

   } 
   m_LstNum = i;
   return 0;
}

int DataVerify::GetrealStr(char* source)
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

void* DataVerify::CallBack(void* lpvd)
{
    DataVerify *p = (DataVerify*)lpvd;

	p->run();
	return p;
}

void  DataVerify::start()
{
	pthread_create(&m_tid, NULL, CallBack, (void *)this);
}

void  DataVerify::join()
{
	if(m_tid)
	{
		pthread_join(m_tid, NULL);
		m_tid = 0;
	}
}


int DataVerify::run()
{
	char szlocal[300] = {0};
	char szremote[300]= {0};
	char szFile[300]  = {0};
	char szMsg[300]   = {0};

	char szlstName[300];
	char szTmpListName[300];

	char *p = NULL;
	int  i  = 0, j = 0, k = 0, ret = 0;

	for (;1;)
	{

		for(i = 0; i < m_LstNum; i++)
		{
			sprintf(szlstName, "%s.Ready", m_LstInf[i].m_CheckLstMName);
			// if *.lst.Finish not exists, continue
			if (access(szlstName, 0))
			{
				continue;
			} 

			FILE *fprLst = fopen(szlstName, "rt");
			if (NULL == fprLst)
			{
				LOG("%s NULL Pointer\n", szlstName);
				continue;
			}

			//读取列表中的文件名存入datafile
			while(fgets(szlocal, 300, fprLst))
			{
				GetrealStr(szlocal);
				p = strrchr(szlocal, '/');
				if(p != NULL)
					strcpy(szFile, p + 1);  //获取纯文件名
				else
					strcpy(szFile, szlocal);
			
				//远程文件名
				if(m_LstInf[i].m_GroupRoot == NULL)
					sprintf(szremote, "%s", szFile);
				else
					sprintf(szremote, "%s/%s", m_LstInf[i].m_GroupRoot, szFile);

				for (j = 0; j < m_LstInf[i].m_RealGroupNum; j++)
				{
					if (m_LstInf[i].m_GroupInf[j].m_SendDataType == DF_SEND_DATA_MOD)
						continue;

					for(k = 0; k < m_LstInf[i].m_GroupInf[j].m_RealServerNum; k++)
					{
						ret = m_LstInf[i].m_GroupInf[j].m_Server[k].checkMD5(szlocal, szremote);
						if(ret)
						{
							snprintf(szMsg, sizeof(szMsg), "[%s:%d]%s data error!", m_LstInf[i].m_GroupInf[j].m_Server[k].getIP().c_str(), 
									m_LstInf[i].m_GroupInf[j].m_Server[k].getPort(), szremote);

							SendSmsNotify(szMsg);
							LOG("%s, ret[%d]", szMsg, ret);
						}
					}//end of for k
				 
				}//end of for j

			}

			//.Finish -> .Verify
			sprintf(szTmpListName, "%s.Verify", m_LstInf[i].m_CheckLstMName);
			int ret = rename(szlstName, szTmpListName);
			if(ret)
				LOG("rename error: %s -> %s, ret [%d], errno: %d\n", szlstName, szTmpListName, ret, errno);

		}//end of for i
		
		
	}//end for (;1;)

	delete p;
	return 0;
}
