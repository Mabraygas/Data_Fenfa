// QSEPSDF_MainControl.cpp: implementation of the CQSEPSDF_MainControl class.
//
//////////////////////////////////////////////////////////////////////

#include "QSEPSDF_MainControl.h"
#include "DayLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define  LOG(format, args...) g_clilog.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

extern DayLog g_clilog;
int MAX_LST_NUM = 0, MAX_GROUP_NUM = 0, MAX_S_NUM_AGROUP = 0, FILE_NUM = 0;

CQSEPSDF_MainControl::CQSEPSDF_MainControl()
{
	m_Que = NULL;
	m_CMem = NULL;
	m_DataF = NULL;
    m_LstNO = new _LSTNO_[MAX_LST_NUM];
}

CQSEPSDF_MainControl::~CQSEPSDF_MainControl()
{
    delete[] m_LstNO;
}

int CQSEPSDF_MainControl::MainInitSys(void **obj,int QueSize,char* Dpath)
{
	int i =0;
	for (i = 0;i<2;i++)
	{
		m_abstract[i] = obj[i];
	}

	m_QueSize = QueSize;
	m_Que     = new QueEvent(m_QueSize);
	m_DataF   = new DataField[m_QueSize];


	for (i = 0;i<m_QueSize;i++)
	{
		m_Que->Push_W(i);
		InitializeCriticalSection(&m_DataF[i].m_Cri);

	}

    m_CMem = new CMemManger();
	//if(m_CMem->InitSys(1000<<20, 50<<20))
	if(m_CMem->InitSys(1<<16, 1<<10))
		return -1;

	m_MemReqSize[1] = SUIT_MEM_FIELD_SIZE;
	m_MemReqSize[0] = BIG_MEM_FIELD_SIZE;

    sprintf(m_DataPath,"%s",Dpath);

	return 0;
}

int CQSEPSDF_MainControl::getQueSize() const
{
	return m_QueSize;
}

char*  CQSEPSDF_MainControl::getDataPath()
{
	if(m_DataPath[0] != '\0')
		return m_DataPath;
	else
		return NULL;
}

void CQSEPSDF_MainControl::FreeSys()
{

	if (m_Que)
	{
		delete m_Que;
		m_Que = NULL;
	}

	if (m_CMem )
	{
		m_CMem->Destory();
		delete m_CMem;
	}

	for (int i = 0;i<m_QueSize;i++)
	{
		DeleteCriticalSection(&m_DataF[i].m_Cri);
	}

	if(m_DataF)
	{
		delete [] m_DataF;
	}

}

int CQSEPSDF_MainControl::GetAFreeField(_DFIELD_* &pData, _DFIELD_& CurData)
{
   
	int QueNO =  m_Que->Pop();
	if (QueNO < 0)
	{
		return -1;
	}
	pData = m_DataF + QueNO;

	pData->m_NO = QueNO;
	//pData->m_DataSize = CurData.m_DataSize;
	//pData->m_DataLength = CurData.m_DataLength;
	pData->m_MemType = CurData.m_MemType;
	//List Name
    //strcpy(pData->m_Listname, CurData.m_Listname);
	//pData->m_ListNameLen = CurData.m_ListNameLen;

	//File Name
	//strcpy(pData->m_Filename, CurData.m_Filename);
	//pData->m_FileNameLen = CurData.m_FileNameLen;

	//pData->m_Cri = CurData.m_Cri;
	//pData->m_FreeSymbole = CurData.m_FreeSymbole;
	//pData->m_FP = CurData.m_FP;
	//pData->m_Key = CurData.m_Key;

	//pData->m_OPNO = CurData.m_OPNO;
	//pData->m_OperateType = CurData.m_OperateType;
	//pData->m_Result= CurData.m_Result;


	if (DF_NOT_NEED_MEM == CurData.m_MemType || CurData.m_PData || 
			(CurData.m_MemType != BIG && CurData.m_MemType != SUITE))
	{
		pData->m_PData = CurData.m_PData;
		pData->m_DataSize = CurData.m_DataSize;
	}else if ((pData->m_MemType == BIG || pData->m_MemType == SUITE) && 
			CurData.m_DataSize > 0 && CurData.m_PData == NULL)
    {
		pData->m_DataSize = (CurData.m_DataSize + m_MemReqSize[pData->m_MemType] - 1)/m_MemReqSize[pData->m_MemType];
		pData->m_PData = m_CMem->Reqmem(pData->m_MemType, pData->m_DataSize);
		if (NULL == pData->m_PData)
		{
			m_Que->Push_W(QueNO);
			return -2;
		}
		
    }

	return 0;

}

int CQSEPSDF_MainControl::FreeAField(_DFIELD_ *pData)
{

	/////////////////////////////////////////////////////
	int No = pData->m_NO;
	//SUITE方式直接释放，不用进临界区
	if(pData->m_MemType == SUITE)
	{
		if(pData->m_PData)
		{
			m_CMem->Freemem(pData->m_MemType, pData->m_PData, pData->m_DataSize);
			pData->m_PData = NULL;
		}
		//pData->m_OperateType = DF_NOT_OPR;
		//pData->m_FreeSymbole = 0;
		m_Que->Push_W(No);
		return 0;
	}

	///////////////////////////////////////////////////////////////
	EnterCriticalSection(&m_DataF[No].m_Cri);
	pData->m_FreeSymbole --;

	if (pData->m_FreeSymbole <= 0)
	{
		if (pData->m_PData && (pData->m_MemType == SUITE || pData->m_MemType == BIG))
		{
			m_CMem->Freemem(pData->m_MemType, pData->m_PData, pData->m_DataSize);
			pData->m_PData = NULL;
		}

		if(pData->m_OperateType == DF_CLOSE_FILE)
		{
			UpdateLstNO(pData->m_OPNO);
		}
		else if(pData->m_OperateType == DF_END_LST)
		{
			ResetLstNO(pData->m_OPNO);
		}
		pData->m_DataSize = 0;
		pData->m_DataLength = 0;
		pData->m_MemType = DF_NOT_NEED_MEM;
		pData->m_Filename[0] = 0;
		pData->m_FileNameLen = 0;
		pData->m_Listname[0] = 0;
		pData->m_ListNameLen = 0;

		//pData->m_Cri = NULL;
		pData->m_FreeSymbole = 0;
		pData->m_FP = NULL;
		pData->m_Key = 0;
		pData->m_NO = -1;
		pData->m_OperateType = DF_NOT_OPR;
	    pData->m_Result= -1;
	    m_Que->Push_W(No);
	}
	LeaveCriticalSection(&m_DataF[No].m_Cri);
		
	return 0;
}

int CQSEPSDF_MainControl::ReadLstNO(const int iLstNum)
{
	char filename[300];
	_LSTNO_* pLstNO = NULL;

	for(int i = 0; i < iLstNum; i++)
	{
		sprintf(filename, "%s.%.3d", DF_LSTNO_FILE_NAME, i);
		pLstNO = m_LstNO + i;

		FILE *fpr = fopen(filename,"rb");
		if (NULL == fpr)
		{
			pLstNO->m_AllSize = 0;
			pLstNO->m_FileNum = 0;
			pLstNO->m_LSTNO = i;
			pLstNO->m_Time = time(NULL);
			continue;
		}

		if(fread(pLstNO, sizeof(_LSTNO_), 1, fpr) != sizeof(_LSTNO_))
		{
			fclose(fpr);
			continue;
		}
		fclose(fpr);
	}
	return 0;
}

int CQSEPSDF_MainControl::UpdateLstNO(const int ClientNO)
{
	_LSTNO_* pLstNO = m_LstNO + ClientNO;
	pLstNO->m_FileNum++;

	char filename[300];
	sprintf(filename, "%s.%.3d.tmp", DF_LSTNO_FILE_NAME, ClientNO);

    FILE *fpw = fopen(filename,"wb");
	if (NULL == fpw)
	{
		LOG("NULL Pointer!\n");
		return -1;
	}

	if (fwrite(pLstNO, 1, sizeof(_LSTNO_), fpw) != sizeof(_LSTNO_))
	{
		fclose(fpw);
		LOG("Write errno:[%d]\n", errno);
		return -2;
	}
	fclose(fpw);
	fpw = NULL;

	char filename1[300];
	
	sprintf(filename1,"%s.%.3d", DF_LSTNO_FILE_NAME, ClientNO);
	if(rename(filename, filename1))
	{
		LOG("rename error:%s -> %s\n", filename, filename1);
		return -3;
	}
	
	return 0;
}

void CQSEPSDF_MainControl::GetLstNO(_LSTNO_* pLstNO, const int GetNO)
{
	memcpy(pLstNO, &m_LstNO[GetNO], sizeof(_LSTNO_));
}

int CQSEPSDF_MainControl::ResetLstNO(const int ClientNO)
{
	_LSTNO_* pLstNO = m_LstNO + ClientNO;

	pLstNO->m_AllSize = 0;
	pLstNO->m_FileNum = 0;
	pLstNO->m_LSTNO = 0;
	pLstNO->m_Time = time(NULL);

	char filename[300];
	int ret = 0;
	sprintf(filename,"%s.%.3d", DF_LSTNO_FILE_NAME, ClientNO);
	ret = remove(filename);
	if(ret)
	{
		LOG("remove %s error, errno:[%d], ret:[%d]\n", filename, errno, ret);
		//printf("remove %s error, errno:[%d], ret:[%d]\n", filename, errno, ret);
	}
	else
	{
		LOG("remove %s\n", filename);
	}

	sprintf(filename,"%s.%.3d.tmp", DF_LSTNO_FILE_NAME, ClientNO);
	remove(filename);
	return 0;
}

