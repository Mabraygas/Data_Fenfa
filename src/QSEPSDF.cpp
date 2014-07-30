// QSEPSDF.cpp : Defines the entry point for the console application.
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include "QSEPSDF_CS.h"
#include "QSEPSDF_SS.h"
#include "QSEPSDF_MainControl.h"
#include "CountNumber.h"

using namespace std;

void usage(int argc, char* argv[])
{
	cout<<"usage:"<<argv[0]<<" [[-c][-s]] [[queue size][port]]"<<endl;
	cout<<argv[0]<<" -p [queue size] [port]          // Client and Server"<<endl;
	cout<<argv[0]<<" -c [queue size]                 // Start client"<<endl;
	cout<<argv[0]<<" -s [port]                       // Start Server"<<endl;
}

static void sigchild_handler(int signo)
{
	int waiter;
	waitpid(-1, &waiter, WNOHANG);
}

int main(int argc, char* argv[])
{

	if(argc < 2 || argc > 4)
	{
		usage(argc, argv);
		return -1;
	}
    int initSuccess = CountNumber(MAX_LST_NUM, MAX_GROUP_NUM, MAX_S_NUM_AGROUP, FILE_NUM);
    CQSEPSDF_CS dfClient;
	CQSEPSDF_SS dfServer;

	CQSEPSDF_MainControl dfMainControl;
	void* aAbstract[2];
	aAbstract[0] = (void*)&dfClient; 
	aAbstract[1] = (void*)&dfServer;

		
	char szDataRoot[100];
	memset(szDataRoot, 0, 100);
	//sprintf(szDataRoot, "/opt/sokuproc/data");
	getcwd(szDataRoot, 100);
		
	int iQueSize = 3000;

	uint16_t port = 51116;
	uint32_t threadnum = 1;
	int ret = 0;
	signal(SIGCHLD, sigchild_handler);

	if(strcmp("-c", argv[1]) == 0)
	{
		if(argc >= 3)
		{
			iQueSize = atoi(argv[2]);
		}
		dfMainControl.MainInitSys(aAbstract, iQueSize , szDataRoot);
		dfClient.InitSys(&dfMainControl);
		ret = dfClient.Start();
		if(ret < 0)
		{
			printf("DFSystem Client Start error:[%d]\n", ret);
		}

	}else if(strcmp("-s", argv[1]) == 0)
	{
		if(argc >= 3)
		{
			port = atoi(argv[2]);
		}

		dfServer.InitSys(port, threadnum);
		dfServer.StartServer();
	}else if(strcmp("-p", argv[1]) == 0)
	{
		
		if(argc >=4)
		{
			iQueSize = atoi(argv[2]);
			port = atoi(argv[3]);

		}
		pid_t Pid= fork();
		if(Pid == 0)
		{
			dfServer.InitSys(port, threadnum);
			dfServer.StartServer();

		}else{
			dfMainControl.MainInitSys(aAbstract, iQueSize , szDataRoot);
			dfClient.InitSys(&dfMainControl);
			dfClient.Start();
		}
	}
	else
	{
		usage(argc, argv);
		return -1;
	}

	while(1)
	{
		sleep(100);
	}

	dfMainControl.FreeSys();
	return 0;
}

