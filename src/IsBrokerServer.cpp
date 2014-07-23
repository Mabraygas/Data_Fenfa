#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <eagle_ex.h>
#include <eagle_epoll_server.h>

#include "Global.h"
#include "protocol.h"
#include "IsBrokerWork.h"
#include "ThreadMd5.h"
#include "DayLog.h"

using namespace std;
using namespace eagle;

#define PROTOCOL_PORT 51116
#define THREAD_NUM    1u
//#define MAX_FILE_NUM  300
#define  LOG(format, args...) g_Srvlog.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

DayLog g_Srvlog;
string          g_QueFile[MAX_FILE_NUM];
QueEvent		g_FilePos(MAX_FILE_NUM); 
QueEvent		g_WorkPos(MAX_FILE_NUM); 

static void usage(int argc, char* argv[])
{
	cout<<"usage:"<<argv[0]<<" [port]]"<<endl;
	cout<<argv[0]<<" [port]                       // Start Server"<<endl;
}

static void sigchild_handler(int signo)
{
	int waiter;
	waitpid(-1, &waiter, WNOHANG);
}

int main(int argc, char* argv[])
{

	if(argc > 2)
	{
		usage(argc, argv);
		return -1;
	}else if(argc==2 && strcmp("-h", argv[1]) == 0)
	{
		usage(argc, argv);
		return -1;

	}

	uint16_t port = 51116;
	uint32_t threadnum = 1;

	g_Srvlog.Init(30, "../log/DFS_IB_Server");

	for(int i = 0; i < MAX_FILE_NUM; i ++)
	{
		g_FilePos.Push_W(i);
	}

	if(argc >= 2)
	{
		port = atoi(argv[1]);
	}

	string   ip   = "0.0.0.0";
	//uint16_t port = PROTOCOL_PORT;
	//uint32_t thread_num     = static_cast<uint32_t>(THREAD_NUM);

	cout << "ip             : " << ip << endl;
	cout << "port           : " << port << endl;
	cout << "thread_num     : " << threadnum << endl;

	signal(SIGCHLD, sigchild_handler);

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

	LOG("============================DFSystem Is Broker Server Start==========================\n");

	server->CreateWorkGroup<IsBrokerWork>("IsBrokerWork",threadnum, protocol_adapter);

	//ThreadMd5* pMd5 = new ThreadMd5;
	//pMd5->start();
	vector<ThreadMd5*> vecMd5;
	for(int i = 0; i < 4; i ++)
	{
		ThreadMd5 *pMd5 = new ThreadMd5;
		pMd5->start();
		vecMd5.push_back(pMd5);
	}

    //运行服务 : 不断地接收请求,循环处理.	
    server->WaitForShutdown();

	for(vector<ThreadMd5*>::iterator itr = vecMd5.begin(); itr!= vecMd5.end(); itr++)
	{
		(*itr)->join();
		delete (*itr);
	}

	//pMd5->join();

	return 0;
}

