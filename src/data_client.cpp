// QSEPSDF.cpp : Defines the entry point for the console application.
#include <iostream>
#include <sys/types.h>

#include "QSEPSDF_CS.h"
#include "QSEPSDF_SS.h"
#include "QSEPSDF_MainControl.h"

using namespace std;

void usage(int argc, char* argv[])
{
	cout<<"usage:"<<argv[0]<<" [queue size]"<<endl;
	cout<<argv[0]<<" [queue size]                 // Start client"<<endl;
}
int main(int argc, char* argv[])
{

	if(argc > 2)
	{
		usage(argc, argv);
		return -1;
	}

	CQSEPSDF_CS dfClient;
	CQSEPSDF_SS dfServer;

	CQSEPSDF_MainControl dfMainControl;

	void* aAbstract[2];
	aAbstract[0] = (void*)&dfClient; 
	aAbstract[1] = (void*)&dfServer;

		
	char szDataRoot[100];
	memset(szDataRoot, 0, 100);
	getcwd(szDataRoot, 100);
		
	int iQueSize = 3000;
	int ret = 0;

	if(argc == 2)
	{
		iQueSize = atoi(argv[1]);
	}

	dfMainControl.MainInitSys(aAbstract, iQueSize , szDataRoot);
	dfClient.InitSys(&dfMainControl);
	ret = dfClient.Start();
	if(ret < 0)
	{
		printf("DFSystem Client Start error:[%d]\n", ret);
	}


	while(1)
	{
		sleep(100);
	}

	dfMainControl.FreeSys();
	return 0;
}

