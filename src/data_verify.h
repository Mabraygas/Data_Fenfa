/**
 * @file DataVerify.h
 * @brief ����У��
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-19
 */

#ifndef _DATA_VERIFY_H_
#define _DATA_VERIFY_H_

#include <string>

#include "QSEPSDF_MainControl.h"
#include "work_client.h"

extern int MAX_LST_NUM, MAX_GROUP_NUM, MAX_S_NUM_AGROUP;
typedef struct VerifyGroup 
{
	int						m_GroupNo;                     //��ǰ��ı��
	int						m_SendDataType;                //���͵���������
	//WorkClient			    m_Server[MAX_S_NUM_AGROUP];    //����������Ϣ
	WorkClient*             m_Server;
    int						m_RealServerNum;               //ʵ�ʵ��ļ�����������Ŀ
    
    VerifyGroup() {
        m_Server = new WorkClient[MAX_S_NUM_AGROUP];
    }
    ~VerifyGroup() {
        delete[] m_Server;
    }
    
}vGroup;

typedef struct VerifyList 
{
	//vGroup	                m_GroupInf[MAX_GROUP_NUM];
	vGroup*                 m_GroupInf;
    int		                m_RealGroupNum;                //ʵ����Ҫ���Ƶ������Ŀ
	char	                m_CheckLstMName[300];          //��ǰ���ڶ���List
	char                    m_GroupLstName[300];           //�������б������
	char                    m_GroupRoot[300];              //����������ļ���·��
    
    VerifyList() {
        m_GroupInf = new vGroup[MAX_GROUP_NUM];
    }
    ~VerifyList() {
        delete[] m_GroupInf;
    }
    
}vList;

class DataVerify  
{
public:
	DataVerify();
	virtual ~DataVerify();

public:
	int                     m_LstNum;
	//vList                   m_LstInf[MAX_LST_NUM];
    vList*                  m_LstInf;

public:
	int                     InitSys(const char* config);
	int                     LoadAllINF(const char* config);

	//��ȡXML
    int                     LoadXml(const char* xmlFile);

	int                     GetrealStr(char* source);

	//�̺߳���
	void                    start();
	int                     run();
	void                    join();

public:
	static void*            CallBack(void* lpvd);

private:
	pthread_t               m_tid;
};

#endif // end of DataVerify
