#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "QSEPSDF_CS.h"
using namespace std;

char* getFileName(char * szFile)
{
	char* p;
	p = strrchr(szFile, '/');
	return p+1;
}

int main(int argc, char* argv[])
{
	char filenameLst[300] = "Eps_List.ready";

	printf("start test server\n");
	FILE *fprLst = fopen(filenameLst,"rt");
	if (NULL == fprLst)
	{
		printf("open %s error\n", filenameLst);
		return -1;
	}

	char TmpBuf[4096];
	size_t readsize = 0;

	CQSEPSDF_CS _cs;
	ServerInf* pServerInfo = new ServerInf;

	pServerInfo->m_ServerNO = 0;
	sprintf(pServerInfo->m_ServerIPStr,"10.103.16.122");
	pServerInfo->m_ServerPort = 51116;
	pServerInfo->m_CurFileFP = NULL;
	pServerInfo->m_ServerHand = new TCPClient(string(pServerInfo->m_ServerIPStr), pServerInfo->m_ServerPort);

	_DFIELD_* pData = new _DFIELD_;
	pData->m_OperateType = DF_START_LST;
	pData->m_DataSize = 0;
	pData->m_NO = 0;
	pData->m_OPNO = 0;
	pData->m_FP = NULL;
	pData->m_PData = TmpBuf;
	sprintf(pData->m_Filename,"Eps_list.txt");

	string result;

	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	result.append(pData->m_Filename, 300);


	_cs.SendAData(result, pData, pServerInfo);

	char filename[300];
	while(fgets(filename, 300, fprLst))
	{	
		_cs.GetrealStr(filename);

		pData->m_OperateType = DF_OPEN_FILE;
		sprintf(pData->m_Filename, "%s", getFileName(filename));
		printf("filename:%s\n",pData->m_Filename);
		

		FILE* fprData = fopen(filename, "rb");
		if(!fprData)
		{
			continue;
		}

		result = "";
		result.append((const char*)&pData->m_OperateType, 4);
		result.append((const char*)&pData->m_NO, 4);
		result.append((const char*)&pData->m_OPNO, 8);
		result.append(pData->m_Filename, 300);

		printf("open file\n");

		_cs.SendAData(result, pData, pServerInfo);


		_DFIELD_ ReadData;
		ReadData.m_NO = 0;
		ReadData.m_OPNO = 0;
		ReadData.m_FP = fprData;
		ReadData.m_DataSize = 0;
		ReadData.m_PData = TmpBuf;
		strcpy(ReadData.m_Filename, pData->m_Filename);
		ReadData.m_OperateType = DF_WRITE_FILE;

		while(!feof(fprData))
		{
			/*if(_cs.ReadAfieldFromFile(&ReadData))
				continue;
			 */
			readsize = fread(ReadData.m_PData, 1, 4096, fprData);
			ReadData.m_DataSize = readsize;

			result = "";
			result.append((const char*)&ReadData.m_OperateType, 4);
			result.append((const char*)&ReadData.m_NO, 4);
			result.append((const char*)&ReadData.m_OPNO, 8);
			result.append((const char*)&ReadData.m_DataSize, 4);
			result.append(ReadData.m_Filename, 300);
			result.append(ReadData.m_PData, ReadData.m_DataSize);

			//printf("send data:%p,%d,result size:%d\n", ReadData.m_FP, ReadData.m_DataSize, (int)result.size());
			_cs.SendAData(result, &ReadData, pServerInfo);
		}
		fclose(fprData);

		pData->m_OperateType = DF_CLOSE_FILE;

		result = "";
		result.append((const char*)&pData->m_OperateType, 4);
		result.append((const char*)&pData->m_NO, 4);
		result.append((const char*)&pData->m_OPNO, 8);
		result.append(pData->m_Filename, 300);

		printf("close file\n");
		_cs.SendAData(result, pData, pServerInfo);

	}
	fclose(fprLst);

	pData->m_OperateType = DF_END_LST;
	sprintf(pData->m_Filename,"Eps_list.txt");

	result = "";
	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_OPNO, 8);
	result.append(pData->m_Filename, 300);

	printf("close list\n");
	_cs.SendAData(result, pData, pServerInfo);



	pData->m_OperateType = DF_DEL_FILE;

	result = "";
	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append(pData->m_Filename, 300);

	_cs.SendAData(result, pData, pServerInfo);

	printf("remove file\n");


}
