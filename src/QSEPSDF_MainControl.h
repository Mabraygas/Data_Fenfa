// QSEPSDF_MainControl.h: interface for the CQSEPSDF_MainControl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _QSEPSDF_MAIN_CONTROL_H
#define _QSEPSDF_MAIN_CONTROL_H

#include <eagle_clientsocket.h>

#ifndef WIN32
#include "Eps_WIN32_LINUX.h"
#endif

#include "CountNumber.h"
#include "MemManger.h"
#include "Global.h"

//List�������ز�����
#define DF_OPEN_FILE		    1 //�ļ���
#define DF_READ_FILE		    2 //�ļ���ȡ
#define DF_WRITE_FILE		    3 //�ļ�д
#define DF_DEL_FILE	     	    4 //�ļ�ɾ��
#define DF_CLOSE_FILE		    5 //�ļ��ر�
#define DF_START_LST		    6 //��ʼLST����
#define DF_END_LST      	    7 //����LST����
#define DF_CREATE_FILE  		8 //�����ļ�
#define DF_CHECK_FILE		    9 //����ļ�
#define DF_RENAME_FILE		    10 //����ļ�
#define DF_SINGLE_FILE  	    11//���䵥���ļ�
#define DF_CHECK_MD5  	        12//MD5У��
#define DF_GET_FILE_MD5         13//MD5У��

#define DF_FILE_CHMOD           14//����������ִ��Ȩ��
#define DF_KILL_PROCESS         15//ɱ������
#define DF_EXEC_PROCESS         16//��������

#define DF_RPC_SHELL            17//Զ�̵���

//
#define DF_LSTNO_FILE_PATH		"lstpath"
#define DF_LSTNO_FILE_NAME		"curlstno.dat"
//
//#define MAX_LST_NUM			    50
#define MAX_FILE_SIZE           6<<20
#define MAX_SEND_SIZE           1048576

//#define MAX_S_NUM_AGROUP	    30	
//#define MAX_GROUP_NUM			3

//����ģʽ��ȡģ������Դ���ݷ�ʽ
#define DF_SEND_DATA_MOD		1	                           //ģ���ķ��Ͷ��е����ݣ����ļ�ģʽ�����ļ����������ļ��Ĺر�ָ����ɿͻ��˷���
#define DF_SEND_DATA_ALL		2	                           //ȫ�ֵķ��Ͷ��е����ݣ���ģʽ�ļ��ر�ָ���ɿͻ��˷���

#define DF_NOT_NEED_MEM		    256	                           //����Ҫ�����ڴ�
#define DF_NOT_OPR			    256                            //��ת����

#define MAX_FILE_NUM            2000                            //�ļ�����������ļ���

extern int MAX_LST_NUM, MAX_GROUP_NUM, MAX_S_NUM_AGROUP, FILE_NUM, MAX_S_NUM_AFILE;

using namespace eagle;

typedef struct ServerInf 
{
	int                     m_ServerNO;                    //���������
	char                    m_ServerIPStr[20];             //��������ַ
	unsigned short          m_ServerPort;                  //�������˿ں�
	TCPClient*              m_ServerHand;                  //���������
	FILE*                   m_CurFileFP;                   //��ǰ�������ڲ����ĶԷ�д�ļ��ľ��

}_SERVER_INF_;

typedef struct GroupInf 
{
    GroupInf()
    {
        m_Server = new _SERVER_INF_[MAX_S_NUM_AGROUP];
    }
    ~GroupInf()
    {
        delete[] m_Server;
    }
	int						m_GroupNo;                     //��ǰ��ı��
	int						m_SendDataType;                //���͵���������
	unsigned __int64		m_CurCheckKey;                 //��ǰ����KEY
	//_SERVER_INF_			m_Server[MAX_S_NUM_AGROUP];    //����������Ϣ
	_SERVER_INF_*           m_Server;
    int						m_RealServerNum;               //ʵ�ʵ��ļ�����������Ŀ
	//CRITICAL_SECTION		m_Cri;                         //��ǰ��������������������ʹ��
    char                    m_GroupRoot[300];              //����������ļ���·��
    char                    m_GroupLstName[300];           //�������б������

}_GINF_;

typedef struct ListInf 
{
    ListInf()
    {
        m_GroupInf = new _GINF_[MAX_GROUP_NUM];
    }
    ~ListInf()
    {
        delete[] m_GroupInf;
    }
    _GINF_*                 m_GroupInf;
	int		                m_RealGroupNum;                //ʵ����Ҫ���Ƶ������Ŀ
	char	                m_CheckLstMName[300];          //��ǰ���ڶ���List
	unsigned __int64        m_CurLstKey;                   //��ǰ�����List���
	char                    m_GroupLstName[300];           //�������б������
	char                    m_GroupRoot[300];              //����������ļ���·��
	FILE*                   m_CurFileFP;                   //��ǰ�������ڲ����ĶԷ�д�ļ��ľ��
}_LST_INF_;

typedef struct SingleFileInf
{
    SingleFileInf() {
        m_Server = new _SERVER_INF_[MAX_S_NUM_AGROUP]; 
    }
    ~SingleFileInf() {
        delete[] m_Server;
    }
    char                    m_GroupRoot[300];
    char                    m_GroupLstName[300];
    FILE*                   m_CurFileFP;
    int                     m_SendDataType;
    unsigned __int64        m_CurCheckKey;
    _SERVER_INF_*           m_Server;
}_SF_INF_;

typedef struct DataField 
{
	int                     m_NO;                          //���ݱ��
	unsigned __int64        m_OPNO;                        //LST�ı��
	int                     m_MemType;                     //�ڴ������
	unsigned __int64        m_Key;                         //�ؼ�Key ������Ƶid
	char                    m_Filename[300];               //�ļ�������
	int                     m_FileNameLen;                 //�ļ�������
	char                    m_Listname[300];               //�ļ�������
	int                     m_ListNameLen;                 //�ļ�������
	int                     m_DataSize;                    //���ݵĴ�С
	int                     m_DataLength;                  //��ʵ���ݳ���
	char*                   m_PData;                       //���ݴ洢�ĵ�ַ
	FILE*                   m_FP;                          //�ļ����
	int                     m_OperateType;                 //�����ı��
	int                     m_Result;                      //�����Ƿ�ɹ��ı�־
    CRITICAL_SECTION        m_Cri;                         //�ٽ�������־
	int                     m_FreeSymbole;                 //�����ڴ�����ͷŵı�־  �ٽ��Ϊ0 ���һ�ξ͵ݼ�
	/*
	~DataField()
	{
		DeleteCriticalSection(m_Cri);
		delete m_Cri;
		m_Cri = NULL;
	}
	 */
}_DFIELD_;

typedef struct LSTNO
{
	 unsigned __int64       m_LSTNO;
	 time_t                 m_Time;
	 int                    m_FileNum;
	 unsigned __int64       m_AllSize;
}_LSTNO_;

class CQSEPSDF_MainControl  
{
public:
	CQSEPSDF_MainControl();
	virtual ~CQSEPSDF_MainControl();
private:
	void*                   m_abstract[2];
    CMemManger*             m_CMem;
	_LSTNO_*                m_LstNO;
    //_LSTNO_                 m_LstNO[MAX_LST_NUM];
	int                     m_ClientNO;
	int                     m_ServerNO;
	int                     m_QueSize;
	int                     m_MemReqSize[2];
	char                    m_DataPath[300];
	

public:
	QueEvent*               m_Que;

	DataField*              m_DataF;

	//int                     m_ClientNUM;

public:
	int                     getQueSize() const;
	char*                   getDataPath();

	int                     MainInitSys(void **obj,int QueSize,char* Dpath);
	void                    FreeSys();
	int                     GetAFreeField(_DFIELD_* &pData, _DFIELD_& CurData);
	int                     FreeAField(_DFIELD_ *pData);

	
	int                     ReadLstNO(const int iLstNum);
	int                     UpdateLstNO(const int ClientNO);
	void                    GetLstNO(_LSTNO_* pLstNO,const int GetNO);
	int                     ResetLstNO(const int ClientNO);


};

#endif // 
