/**
 * @file CQSEPSDF_CS.h
 * @brief �ַ��ͻ���
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-09-24
 */

#ifndef _QSEPSDF_CS_H_
#define _QSEPSDF_CS_H_

#include <string>
#include "uks_mem.h"

#include "QSEPSDF_MainControl.h"
#include "Global.h"
#include "Eps_WIN32_LINUX.h"


class CQSEPSDF_CS  
{
public:
	CQSEPSDF_CS();
	virtual ~CQSEPSDF_CS();

public:
	CQSEPSDF_MainControl* m_MainControl;
	int m_LstNum;
	//_LST_INF_ m_LstInf[MAX_LST_NUM];
    _LST_INF_* m_LstInf;
    //_GINF_* m_GInf;
	//CRITICAL_SECTION m_GroupCri[MAXLSTNUM];
	CRITICAL_SECTION m_MainCri;
	int m_ThreadNO;

	QueEvent**** m_pDataQue;
    //QueEvent* m_pDataQue[MAX_LST_NUM][MAX_GROUP_NUM][MAX_S_NUM_AGROUP];
public:

	int getLocalIP(char *ip);
	int InitSys(CQSEPSDF_MainControl* Main);

	//��ȡ�����ļ�����Ϣ ���ݹ�����Ŀ�Ĳ�ͬ������д
	int                     LoadAllINF();

/*
	int                     ReadLstNO();
	int                     UpdateLstNO(const int ClientNO);
	void                    GetLstNO(_LSTNO_* pLstNO,const int GetNO);
	int                     ResetLstNO(const int ClientNO);
*/
	//��ȡXML
    int LoadXml(const char* xmlFile);
	int GetrealStr(char* source);
	virtual int ReadAfieldFromFile(_DFIELD_*pData );//-1,�ļ����� -2 �ļ���ȡʧ��
	int BuildDataToStr(_DFIELD_* pData, std::string& result);


	//���紫���õ�����庯��
	int CreateALst(_DFIELD_* pData,ServerInf * pServer);
	int EndALst(_DFIELD_* pData,ServerInf * pServer);
	int OpenAFile(_DFIELD_* pData,ServerInf * pServer);
	int CloseAFile(_DFIELD_* pData,ServerInf * pServer);
	int WriteADataToFile(_DFIELD_* pData,ServerInf * pServer);
	int CreateAFile(_DFIELD_* pData,ServerInf * pServer);

	//Main Work������Ҫ�ĺ���
	int Work_StartList(const int32_t CurThreadNo, _DFIELD_* TmpData);
	int Work_OpenFile(const int32_t CurThreadNo, _DFIELD_* TmpData);
	int Work_CloseFile(const int32_t CurThreadNo, _DFIELD_* TmpData);
	int Work_CloseList(const int32_t CurThreadNo, _DFIELD_* TmpData);
	uint64_t Work_ModData(const int32_t CurThreadNo, const int Gidx, _DFIELD_* TmpReadData, RStreamBuf* rstream);  //����ȡģ����
	uint64_t Work_ALLData(const int32_t CurThreadNo, const int Gidx, _DFIELD_* TmpReadData, FILE* fprData);  //����������
	int Work_CreateAFile(const int32_t CurThreadNo, _DFIELD_* TmpData);

	int Work_Verify(const int32_t CurThreadNo);
	int Work_MoveMD5File(const int32_t CurThreadNo);
	int BuildMd5(const char* szFile, std::string& strmd5, char* pBuf);


	int SendAData(std::string &result, _DFIELD_* PData, ServerInf* pServerInfo);
	//�̺߳���
	int MainWork();
	int Main_Command(int LNo, int GNo, int SNo);

	
	//���������߳�
	int Start();
public:

	static void* Thread_MainWork(void* lpvd) ;
	static void* Thread_Main_Command(void* lpvd) ;

private:
	char                    m_szRoot[300];
	//_LSTNO_                 m_LstNO[MAX_LST_NUM];
	char                    m_szLocalIp[20];
};

typedef struct LstGrpSrv
{
	int LstNO;
	int GrpNO;
	int SrvNO;
	CQSEPSDF_CS* lpvd;
	LstGrpSrv(int LNo, int GNo, int SNo, CQSEPSDF_CS* lpvoid):LstNO(LNo),GrpNO(GNo),SrvNO(SNo)
	{
		lpvd = lpvoid;
	}

}_LGS_;

#endif // end of QSEPSDF_CS
