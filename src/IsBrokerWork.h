#ifndef _IS_BROKER_WORK_H_
#define _IS_BROKER_WORK_H_
/**
 * @file IsBrokerServer.h
 * @brief IS and Broker接收发送处理线程
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
	 * @brief 默认构造函数
	 */
	IsBrokerWork() {}


	~IsBrokerWork();


protected:

	/**
	 * @brief 线程启动前的初始化.
	 */
	virtual void initialize();

protected:

	/**
	 * @brief 钩子函数
	 *
	 * @param [recv] : adapter接收队列中的元素
	 */
	virtual void work(const RecvData &recv);

	/**
	 * @brief 处理超时数据
	 * 数据在Adapter接收数据队列中的时间已经超过允许值.
	 * 是否超时,是在WorkImp()中判断的.
	 * 默认调用close(),直接关闭连接.
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	virtual void WorkTimeout(const RecvData &recv);

	/**
	 * @brief 钩子函数
	 *
	 * @param [szFile] : 要生成md5的文件名
	 */
	int BuildMd5(const char* szFile, string& result, char* pBuf);

	/**
	 * @brief 构造返回包
	 *
	 * @param [msg]  :  构造的返回包
	 * @param [pData]:  传入需要构造返回包的信息
	 */
    void BuildResponsePack(string& msg, _DFIELD_* pData);

	/**
	 * @brief 创建List,存入m_mapList
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
    void CreateAList(const RecvData &recv);

	/**
	 * @brief 关闭List,并从m_mapList中删除
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void CloseAList(const RecvData &recv);

	/**
	 * @brief 打开文件,并将文件名写入对应的List，将文件名与文件指针存入m_mapNameFile
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void OpenAFile(const RecvData &recv);

	/**
	 * @brief 关闭文件,并将文件名从m_mapNameFile中删除
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void CloseAFile(const RecvData &recv);

	/**
	 * @brief 将数据写入对应的文件
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void WriteToFile(const RecvData &recv);

	/**
	 * @brief 根据指定的文件名删除文件
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void RemoveFile(const RecvData &recv);

	/**
	 * @brief 创建文件
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void CreateAFile(const RecvData &recv);

	/**
	 * @brief 检测文件存在
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void CheckAFile(const RecvData &recv);

    /**
	 * @brief Rename File
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void RenameFile(const RecvData &recv);

	/**
	 * @brief 接收单个文件
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void RecvSingleFile(const RecvData &recv);

	/**
	 * @brief 校验某个文件MD5值
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void CheckMD5(const RecvData &recv);

	/**
	 * @brief 获取某个文件MD5值
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void GetFileMD5(const RecvData &recv);

	/**
	 * @brief 给文件增加执行权限
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void FileChmod(const RecvData &recv);

	/**
	 * @brief 杀死进程
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void KillProcess(const RecvData &recv);

	/**
	 * @brief 启动进程
	 *
	 * @param [recv] : Adapter接收队列中的元素
	 */
	void ExecProcess(const RecvData &recv);

	/**
	 * @brief RPC
	 *
	 * @param [recv] : 远程调用
	 */
	void RPCShell(const RecvData &recv);

private:

	//存储文件名与文件指针的映射
	map<string, FILE*> m_mapNameFile;

	//存储List文件名与文件指针的映射
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

