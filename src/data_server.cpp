#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include "QSEPSDF_SS.h"

using namespace std;

void usage(int argc, char* argv[])
{
	cout<<"usage:"<<argv[0]<<"[port]]"<<endl;
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
	}

	CQSEPSDF_SS dfServer;

	uint16_t port = 11111;
	uint32_t threadnum = 1;

	if(argc == 2)
	{
		port = atoi(argv[1]);
	}
	signal(SIGCHLD, sigchild_handler);

	dfServer.InitSys(port, threadnum);
	dfServer.StartServer();

	return 0;
}

