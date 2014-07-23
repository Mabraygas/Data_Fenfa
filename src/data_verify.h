/**
 * @file DataVerify.h
 * @brief 数据校验
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
	int						m_GroupNo;                     //当前组的编号
	int						m_SendDataType;                //发送的数据类型
	//WorkClient			    m_Server[MAX_S_NUM_AGROUP];    //服务器的信息
	WorkClient*             m_Server;
    int						m_RealServerNum;               //实际的文件服务器的数目
    
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
    int		                m_RealGroupNum;                //实际需要控制的组的数目
	char	                m_CheckLstMName[300];          //当前正在读的List
	char                    m_GroupLstName[300];           //服务器列表的名称
	char                    m_GroupRoot[300];              //存入服务器文件的路径
    
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

	//读取XML
    int                     LoadXml(const char* xmlFile);

	int                     GetrealStr(char* source);

	//线程函数
	void                    start();
	int                     run();
	void                    join();

public:
	static void*            CallBack(void* lpvd);

private:
	pthread_t               m_tid;
};

#endif // end of DataVerify
