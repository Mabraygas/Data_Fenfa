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

//List处理的相关操作码
#define DF_OPEN_FILE		    1 //文件打开
#define DF_READ_FILE		    2 //文件读取
#define DF_WRITE_FILE		    3 //文件写
#define DF_DEL_FILE	     	    4 //文件删除
#define DF_CLOSE_FILE		    5 //文件关闭
#define DF_START_LST		    6 //开始LST传输
#define DF_END_LST      	    7 //结束LST传输
#define DF_CREATE_FILE  		8 //创建文件
#define DF_CHECK_FILE		    9 //检测文件
#define DF_RENAME_FILE		    10 //检测文件
#define DF_SINGLE_FILE  	    11//传输单个文件
#define DF_CHECK_MD5  	        12//MD5校验
#define DF_GET_FILE_MD5         13//MD5校验

#define DF_FILE_CHMOD           14//给程序增加执行权限
#define DF_KILL_PROCESS         15//杀死进程
#define DF_EXEC_PROCESS         16//启动进程

#define DF_RPC_SHELL            17//远程调用

//
#define DF_LSTNO_FILE_PATH		"lstpath"
#define DF_LSTNO_FILE_NAME		"curlstno.dat"
//
//#define MAX_LST_NUM			    50
#define MAX_FILE_SIZE           6<<20
#define MAX_SEND_SIZE           1048576

//#define MAX_S_NUM_AGROUP	    30	
//#define MAX_GROUP_NUM			3

//传输模式：取模和完整源数据方式
#define DF_SEND_DATA_MOD		1	                           //模数的发送队列的内容，该文件模式传输文件，服务器文件的关闭指令不能由客户端发送
#define DF_SEND_DATA_ALL		2	                           //全局的发送队列的内容，该模式文件关闭指令由客户端发送

#define DF_NOT_NEED_MEM		    256	                           //不需要开闭内存
#define DF_NOT_OPR			    256                            //空转操作

#define MAX_FILE_NUM            2000                            //文件名队列最大文件数

extern int MAX_LST_NUM, MAX_GROUP_NUM, MAX_S_NUM_AGROUP, FILE_NUM, MAX_S_NUM_AFILE;

using namespace eagle;

typedef struct ServerInf 
{
	int                     m_ServerNO;                    //服务器编号
	char                    m_ServerIPStr[20];             //服务器地址
	unsigned short          m_ServerPort;                  //服务器端口号
	TCPClient*              m_ServerHand;                  //服务器句柄
	FILE*                   m_CurFileFP;                   //当前服务器在操作的对方写文件的句柄

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
	int						m_GroupNo;                     //当前组的编号
	int						m_SendDataType;                //发送的数据类型
	unsigned __int64		m_CurCheckKey;                 //当前检验KEY
	//_SERVER_INF_			m_Server[MAX_S_NUM_AGROUP];    //服务器的信息
	_SERVER_INF_*           m_Server;
    int						m_RealServerNum;               //实际的文件服务器的数目
	//CRITICAL_SECTION		m_Cri;                         //当前组的锁，用于做归零计算使用
    char                    m_GroupRoot[300];              //存入服务器文件的路径
    char                    m_GroupLstName[300];           //服务器列表的名称

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
	int		                m_RealGroupNum;                //实际需要控制的组的数目
	char	                m_CheckLstMName[300];          //当前正在读的List
	unsigned __int64        m_CurLstKey;                   //当前处理的List编号
	char                    m_GroupLstName[300];           //服务器列表的名称
	char                    m_GroupRoot[300];              //存入服务器文件的路径
	FILE*                   m_CurFileFP;                   //当前服务器在操作的对方写文件的句柄
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
	int                     m_NO;                          //数据编号
	unsigned __int64        m_OPNO;                        //LST的编号
	int                     m_MemType;                     //内存的类型
	unsigned __int64        m_Key;                         //关键Key 例如视频id
	char                    m_Filename[300];               //文件的名称
	int                     m_FileNameLen;                 //文件名长度
	char                    m_Listname[300];               //文件的名称
	int                     m_ListNameLen;                 //文件名长度
	int                     m_DataSize;                    //数据的大小
	int                     m_DataLength;                  //真实数据长度
	char*                   m_PData;                       //数据存储的地址
	FILE*                   m_FP;                          //文件句柄
	int                     m_OperateType;                 //操作的编号
	int                     m_Result;                      //操作是否成功的标志
    CRITICAL_SECTION        m_Cri;                         //临界点加锁标志
	int                     m_FreeSymbole;                 //数据内存可以释放的标志  临界点为0 完成一次就递减
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
