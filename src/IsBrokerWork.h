#ifndef _IS_BROKER_WORK_H_
#define _IS_BROKER_WORK_H_
/**
 * @file IsBrokerServer.h
 * @brief IS and Broker���շ��ʹ����߳�
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-30
 */

#include <iostream>
#include <sstream>
#include <fstream>

#include <string>
#include <sys/time.h>
#include <map>

#include <eagle_ex.h>
#include <eagle_epoll_server.h>

#include "QSEPSDF_MainControl.h"
#include "protocol.h"
#include "md5.h"

using namespace std;
using namespace eagle;

class IsBrokerWork : public Work
{
public:

	/**
	 * @brief Ĭ�Ϲ��캯��
	 */
	IsBrokerWork() {}


	~IsBrokerWork();


protected:

	/**
	 * @brief �߳�����ǰ�ĳ�ʼ��.
	 */
	virtual void initialize();

protected:

	/**
	 * @brief ���Ӻ���
	 *
	 * @param [recv] : adapter���ն����е�Ԫ��
	 */
	virtual void work(const RecvData &recv);

	/**
	 * @brief ����ʱ����
	 * ������Adapter�������ݶ����е�ʱ���Ѿ���������ֵ.
	 * �Ƿ�ʱ,����WorkImp()���жϵ�.
	 * Ĭ�ϵ���close(),ֱ�ӹر�����.
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	virtual void WorkTimeout(const RecvData &recv);

	/**
	 * @brief ���Ӻ���
	 *
	 * @param [szFile] : Ҫ����md5���ļ���
	 */
	int BuildMd5(const char* szFile, string& result, char* pBuf);

	/**
	 * @brief ���췵�ذ�
	 *
	 * @param [msg]  :  ����ķ��ذ�
	 * @param [pData]:  ������Ҫ���췵�ذ�����Ϣ
	 */
    void BuildResponsePack(string& msg, _DFIELD_* pData);

	/**
	 * @brief ����List,����m_mapList
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
    void CreateAList(const RecvData &recv);

	/**
	 * @brief �ر�List,����m_mapList��ɾ��
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void CloseAList(const RecvData &recv);

	/**
	 * @brief ���ļ�,�����ļ���д���Ӧ��List�����ļ������ļ�ָ�����m_mapNameFile
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void OpenAFile(const RecvData &recv);

	/**
	 * @brief �ر��ļ�,�����ļ�����m_mapNameFile��ɾ��
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void CloseAFile(const RecvData &recv);

	/**
	 * @brief ������д���Ӧ���ļ�
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void WriteToFile(const RecvData &recv);

	/**
	 * @brief ����ָ�����ļ���ɾ���ļ�
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void RemoveFile(const RecvData &recv);

	/**
	 * @brief �����ļ�
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void CreateAFile(const RecvData &recv);

	/**
	 * @brief ����ļ�����
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void CheckAFile(const RecvData &recv);

    /**
	 * @brief Rename File
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void RenameFile(const RecvData &recv);

	/**
	 * @brief ���յ����ļ�
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void RecvSingleFile(const RecvData &recv);

	/**
	 * @brief У��ĳ���ļ�MD5ֵ
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void CheckMD5(const RecvData &recv);

	/**
	 * @brief ��ȡĳ���ļ�MD5ֵ
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void GetFileMD5(const RecvData &recv);

	/**
	 * @brief ���ļ�����ִ��Ȩ��
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void FileChmod(const RecvData &recv);

	/**
	 * @brief ɱ������
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void KillProcess(const RecvData &recv);

	/**
	 * @brief ��������
	 *
	 * @param [recv] : Adapter���ն����е�Ԫ��
	 */
	void ExecProcess(const RecvData &recv);

	/**
	 * @brief RPC
	 *
	 * @param [recv] : Զ�̵���
	 */
	void RPCShell(const RecvData &recv);

private:

	//�洢�ļ������ļ�ָ���ӳ��
	map<string, FILE*> m_mapNameFile;

	//�洢List�ļ������ļ�ָ���ӳ��
	map<string, FILE*> m_mapList;

	multimap<string, pid_t> m_mapProcPid;

	pthread_mutex_t m_MainCri;

	string m_strListName;
	string m_strRoot;
	time_t m_TimeOut;
	char*  m_pBuf;
	//void *m_para;
};

#endif

