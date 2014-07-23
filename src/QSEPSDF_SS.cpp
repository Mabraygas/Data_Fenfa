/**
 * @file QSEPSDF_SS.cpp
 * @brief 服务器入口
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-10-11
 */


#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include <eagle_ex.h>
#include <eagle_epoll_server.h>

#include "Global.h"
#include "protocol.h"
#include "QSEPSDF_SSWork.h"

#include "QSEPSDF_SS.h"
#include "DayLog.h"

using namespace std;
using namespace eagle;

#define PROTOCOL_PORT 51116
#define THREAD_NUM 1u
#define  LOG(format, args...) g_Srvlog.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

DayLog          g_Srvlog;
string          g_QueFile[MAX_FILE_NUM];
QueEvent		g_FilePos(MAX_FILE_NUM); 
QueEvent		g_WorkPos(MAX_FILE_NUM); 


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQSEPSDF_SS::CQSEPSDF_SS()
{
	//m_pthreadMd5 = new ThreadMd5;
}

CQSEPSDF_SS::~CQSEPSDF_SS()
{
	/*
	if(m_pthreadMd5)
		delete m_pthreadMd5;
	 */
}

int CQSEPSDF_SS::InitSys(uint16_t port, uint32_t threadnum)
{	
	this->port = port;
	this->threadnum = threadnum;
	g_Srvlog.Init(30, "../log/DFS_Server");

	for(int i = 0; i < MAX_FILE_NUM; i ++)
	{
		g_FilePos.Push_W(i);
	}
	return 0;
}

int CQSEPSDF_SS::StartServer()
{

	string   ip   = "0.0.0.0";
	//uint16_t port = PROTOCOL_PORT;
	//uint32_t thread_num     = static_cast<uint32_t>(THREAD_NUM);

	cout << "ip             : " << ip << endl;
	cout << "port           : " << port << endl;
	cout << "thread_num     : " << threadnum << endl;

	//服务器
    EpollServerPtr server = new EpollServer();

    //EagleProtocol适配器
    BindAdapterPtr protocol_adapter = new BindAdapter(server);

    //IP,端口,5分钟超时时间,TCP.
	//客户端在5分钟时间之内若没有"活动",则服务器会删除该连接.
	//这里的"活动"包括:Connecton对象读取client发过来的数据(EPOLLIN);Connecton对象发送数据(EPOLLOUT).
    Endpoint protocol_local(ip,port,20*60*1000,true);
    protocol_adapter->setEndpoint(protocol_local);

    //使用Eagle协议
    protocol_adapter->setProtocol(Protocol::ParseProtocol);
	//设置所绑定的端口上最多能够同时服务client的数目,默认为1024.
    protocol_adapter->setMaxConns(1024);
    //设置接收数据的队列rbuffer_的最大容量,默认为10*1024(个数据包).
    protocol_adapter->setQueueCapacity(10240);
    //设置消息超时时间(从入队列到出队列间隔),单位是毫秒,默认是60秒(60000ms).
    protocol_adapter->setQueueTimeout(60*1000);

	//创建处理线程组,并将适配器加入服务器.

	LOG("==================================DF System Server Start==============================\n");

	server->CreateWorkGroup<QSEPSDF_SSWork>("QSEPSDF_SSWork",threadnum, protocol_adapter);
	
	//build md5
	//m_pthreadMd5->start();
	vector<ThreadMd5*> vecMd5;
	for(int i = 0; i < 4; i ++)
	{
		ThreadMd5 *pMd5 = new ThreadMd5;
		pMd5->start();
		vecMd5.push_back(pMd5);
	}

	//运行服务 : 不断地接收请求,循环处理.
	server->WaitForShutdown();

	//m_pthreadMd5->join();

	for(vector<ThreadMd5*>::iterator itr = vecMd5.begin(); itr!= vecMd5.end(); itr++)
	{
		(*itr)->join();
		delete (*itr);
	}

	return 0;

}
