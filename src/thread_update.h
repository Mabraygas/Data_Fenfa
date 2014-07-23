/**
 * @file thread_update.h
 * @brief ȫ������
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-12-07
 */

#ifndef _THREAD_UPDATE_H_
#define _THREAD_UPDATE_H_

#include <string>

#include "QSEPSDF_MainControl.h"
#include "work_client.h"

#define ALL_UPDATE_SYMBOLE	    "AllUpdateSymbole.dat"
#define DONOT_MERGE_PLUS1M 	    "UpdateServiceSymbole.dat" 
#define EXPORT_END_ON_N         "ExportEndOnN.dat"    //Need change if needed
#define BASE_INI                "base.ini"

#define MAX_SERVER_CNT          3    //Max Server Count
#define MAX_GRP_CNT             3    //Max Group Count

#define BROKER_IP_ONE                 "10.105.16.109" //"10.103.16.92"
#define BROKER_IP_TWO                 "10.105.16.110" //"10.103.16.124"

typedef struct BaseServer 
{
	uint16_t				m_ServerID;                    //��ǰ��ı��
	WorkClient			    m_Client;                      //����������Ϣ
	char                    m_RemoteRoot[300];             //Զ�̶�����Ŀ¼
}BServer;

typedef struct BaseGroup 
{
	uint16_t                m_iKey;
	uint16_t	            m_ServerCount;                //ʵ����Ҫ���Ƶ������Ŀ
	BServer	                m_Server[MAX_SERVER_CNT];
	char	                m_LocalRoot[300];             //���ع���Ŀ¼
}BGroup;

class ThreadUpdate  
{
public:
	ThreadUpdate();
	virtual ~ThreadUpdate();

public:
	int                     m_GrpNum;
	BGroup                  m_aGrp[MAX_GRP_CNT];

public:
	int                     InitSys(const char* config, int iUpdateHour);

	int                     LoadAllINF(const char* config);

	//��ȡXML
    int                     LoadXml(const char* xmlFile);

	int                     GetrealStr(char* source);
	int                     TouchFile(const char * file);
	int                     getLocalIP(char * ip);

	//�̺߳���
	void                    start();
	int                     run();
	void                    join();

public:
	static void*            CallBack(void* lpvd);

private:
	int                     m_iUpdateHour;
	pthread_t               m_tid;
	char                    m_szLocalIP[20];
	char                    m_aBrokerIP[4][20];
	int                     m_dwbrokerCnt;
};

#endif // end of ThreadUpdate
