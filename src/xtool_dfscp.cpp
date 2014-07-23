/**
 * @file xtool_dfscp.cpp
 * @brief 分发scp工具
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-12
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "work_client.h"
#include "DayLog.h"

#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)

DayLog g_log;
static char g_szFile[100] = {0};
static char g_szList[100] = {0};
static char g_szRoot[100] = {0};
static char g_szData[100] = {0};
static bool g_bport = false;

using namespace std;

static void usage(const char* argv0)
{
	printf("usage:\n");
	printf("%s -h                                                            //help\n", argv0);
	printf("%s -A -s [ip] {-p [port]} {-f} [File] {-d} [short data]          //Create a File\n", argv0);
	printf("%s -E -s [ip] {-p [port]} {-f} [File]                            //Check File if exist\n", argv0);
	printf("%s -D -s [ip] {-p [port]} {-f} [File]                            //Remove File\n", argv0);
	printf("%s -F -s [ip] {-p [port]} {-f} [File] {-r} [Path]                //Send File\n", argv0);
	printf("%s -L -s [ip] {-p [port]} {-l} [List] {-r} [Path]                //send List\n", argv0);
	printf("%s -G -s [ip] {-p [port]} {-f} [File]                            //get MD5 of File\n", argv0);
	printf("%s -C -s [ip] {-p [port]} {-f} [Local File] {-d} [Remote File]   //check MD5\n", argv0);
	printf("%s -R -s [ip] {-p [port]} {-f} [Src File] {-d} [Des File]        //rename Src File to Des File\n", argv0);
	printf("%s -M -s [ip] {-p [port]} {-f} [File]                            //u+x mask File\n", argv0);
	printf("%s -K -s [ip] {-p [port]} {-f} [File]                            //kill process named File\n", argv0);
	printf("%s -S -s [ip] {-p [port]} {-f} [File]                            //Start process named File\n", argv0);

	printf("%s -P -s [ip] {-p [port]} [shell-command]                        //RPC\n", argv0);

}

static void	renameFile(WorkClient *pwc, int argc, char* argv[])
{
	if(g_szFile[0] == 0 && g_szData[0] == 0)
	{
		if(g_bport)
		{
			if(argc == 4)
			{
				//[program] -R -s 10.10.10.10 -p 51116 [src] [dst]
				strcpy(g_szFile, argv[2]);
				strcpy(g_szData, argv[3]);
			}
		}
		else
		{
			if(argc == 2)
			{
				//[program] -R -s 10.10.10.10 [src] [dst]
				strcpy(g_szFile, argv[0]);
				strcpy(g_szData, argv[1]);
			}

		}

	}

	if(*g_szFile == 0 || *g_szData == 0)
	{
		printf("input error!\n");
		return;
	}

	int ret = pwc->renameFile(g_szFile, g_szData);

	if(ret)
		printf("rename %s to %s fail\n", g_szFile, g_szData);
	else
		printf("rename %s to %s success\n", g_szFile, g_szData);

}
static void createFile(WorkClient *pwc, int argc, char* argv[])
{

	int ret = 0;
	int iLen = 0;
	if(g_szFile[0] == 0 && g_szData[0] == 0)
	{
		if(g_bport)
		{
			if(argc == 3)
			{
				//[program] -A -s 10.10.10.10 -p 51116 [File]
				//createFile(&wclient, argv[6], NULL);
				ret = pwc->createOneFile(argv[2], NULL, 0);
				strcpy(g_szFile, argv[2]);
			}
			else if(argc == 4)
			{
				//[program] -A -s 10.10.10.10 -p 51116 [File] [Data]
				iLen = strlen(argv[3]);
				ret = pwc->createOneFile(argv[2], argv[3], iLen);
				strcpy(g_szFile, argv[2]);
				//createFile(&wclient, argv[6], argv[7]);
			}
		}
		else
		{
			if(argc == 1)
			{
				//[program] -A -s 10.10.10.10 [File] 
				//createFile(&wclient, argv[4], NULL);
				ret = pwc->createOneFile(argv[0], NULL, 0);

				strcpy(g_szFile, argv[0]);
			}
			else if(argc == 2)
			{

				//[program] -A -s 10.10.10.10 [File] [Data]
				//createFile(&wclient, argv[4], argv[5]);
				iLen = strlen(argv[1]);
				ret = pwc->createOneFile(argv[0], argv[1], iLen);
				strcpy(g_szFile, argv[0]);
			}
		}
	}
	else if(g_szFile[0] != 0 && g_szData[0] != 0)
	{

		//[program] -A -s 10.10.10.10 -p 51116 -f [File] -d [Data]
		//createFile(&wclient, g_szFile, g_szData);
		iLen = strlen(g_szData);
		ret = pwc->createOneFile(g_szFile, g_szData, iLen);
	}
	else if(g_szFile[0] != 0 && g_szData[0] == 0)
	{
		//[program] -A -s 10.10.10.10 -p 51116 -f [File] 
		//createFile(&wclient, g_szFile, NULL);
		ret = pwc->createOneFile(g_szFile, NULL, 0);
	}

	if(ret < 0)
		printf("Create File %s error,ret:[%d]", g_szFile, ret);
	else
		printf("Create File %s success\n", g_szFile);

}

static void	checkFile(WorkClient *pwc, int argc, char* argv[])
{
	int ret = 0;
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -E -s 10.10.10.10 -p 51116 [File] 
			//ret = pwc->checkOneFile(argv[2]);
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -E -s 10.10.10.10 [File] 
			//ret = pwc->checkOneFile(argv[0]);
			strcpy(g_szFile, argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	ret = pwc->checkOneFile(g_szFile);
	if(ret)
		printf("%s not exists\n", g_szFile);
	else
		printf("%s exists\n", g_szFile);

}

static void removeFile(WorkClient *pwc, int argc, char* argv[])
{
	int ret = 0;
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -D -s 10.10.10.10 -p 51116 [File] 
			//ret = pwc->removeOneFile(argv[2]);
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -D -s 10.10.10.10 [File] 
			strcpy(g_szFile, argv[0]);
			//ret = pwc->removeOneFile(argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	ret = pwc->removeOneFile(g_szFile);

	if(ret)
		printf("%s remove error\n", g_szFile);
	else
		printf("%s remove success\n", g_szFile);

}

static void sendFile(WorkClient *pwc, int argc, char* argv[])
{
	if(g_szFile[0] == 0 && g_szRoot[0] == 0)
	{
		if(g_bport)
		{
			if(argc == 3)
			{
				//[program] -F -s 10.10.10.10 -p 51116 [File]
				//sendFile(&wclient, argv[6], NULL);
				strcpy(g_szFile, argv[2]);
			}
			else if(argc == 4)
			{
				//[program] -F -s 10.10.10.10 -p 51116 [File] [Root]
				strcpy(g_szFile, argv[2]);
				strcpy(g_szRoot, argv[3]);
			}
		}
		else
		{
			if(argc == 1)
			{
				//[program] -F -s 10.10.10.10 [File] 
				strcpy(g_szFile, argv[0]);
			}
			else if(argc == 2)
			{
				//[program] -F -s 10.10.10.10 [File] [Root]
				strcpy(g_szFile, argv[0]);
				strcpy(g_szRoot, argv[1]);
			}

		}

	}
    
    if(*g_szFile == 0)
    {
		printf("File Name is NULL!\n");
		return;
    }

    int ret = 0;

	if(g_szRoot[0] != 0)
	{
		//[program] -F -s 10.10.10.10 -p 51116 -f [File] -r [Root]
		ret = pwc->sendFile(g_szFile, g_szRoot);
	}
	else
	{
		//[program] -F -s 10.10.10.10 -p 51116 -f [File] 
		ret = pwc->sendFile(g_szFile, NULL);
	}

	if(ret)
		printf("send %s error\n", g_szFile);
	else
		printf("send %s success\n", g_szFile);

}
static void sendList(WorkClient *pwc, int argc, char* argv[])
{
	if(g_szList[0] == 0 && g_szRoot[0] == 0)
	{
		if(g_bport)
		{
			if(argc == 3)
			{
				//[program] -L -s 10.10.10.10 -p 51116 [List]
				strcpy(g_szList, argv[2]);
			}
			else if(argc == 4)
			{
				//[program] -L -s 10.10.10.10 -p 51116 [List] [Root]
				strcpy(g_szList, argv[2]);
				strcpy(g_szRoot, argv[3]);
			}
		}
		else
		{
			if(argc == 1)
			{
				//[program] -L -s 10.10.10.10 [List] 
				strcpy(g_szList, argv[0]);
			}
			else if(argc == 2)
			{
				//[program] -L -s 10.10.10.10 [List] [Root]
				strcpy(g_szList, argv[0]);
				strcpy(g_szRoot, argv[1]);
			}

		}

	}
    
    if(*g_szList == 0)
    {
		printf("File Name is NULL!\n");
		return;
    }

    int ret = 0;

	if(g_szRoot[0] != 0)
	{
		ret = pwc->sendList(g_szList, g_szRoot);
	}
	else
	{
		ret = pwc->sendList(g_szList, NULL);
	}


	if(ret)
		printf("send List %s error\n", g_szList);
	else
		printf("send List %s success\n", g_szList);

}

static void getMD5(WorkClient *pwc, int argc, char* argv[])
{
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -G -s 10.10.10.10 -p 51116 [File] 
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -G -s 10.10.10.10 [File] 
			strcpy(g_szFile, argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	string result;
	int ret = pwc->getMD5(g_szFile, result);

	if(ret)
		printf("get %s md5 error\n", g_szFile);
	else
		printf("%s md5: %s\n", g_szFile, result.c_str());

}

static void checkMD5(WorkClient *pwc, int argc, char* argv[])
{
	if(g_szFile[0] == 0 && g_szData[0] == 0)
	{
		if(g_bport)
		{
			if(argc == 4)
			{
				//[program] -C -s 10.10.10.10 -p 51116 [src] [dst]
				strcpy(g_szFile, argv[2]);
				strcpy(g_szData, argv[3]);
			}
		}
		else
		{
			if(argc == 2)
			{
				//[program] -C -s 10.10.10.10 [src] [dst]
				strcpy(g_szFile, argv[0]);
				strcpy(g_szData, argv[1]);
			}

		}

	}

	if(*g_szFile == 0 || *g_szData == 0)
	{
		printf("input error!\n");
		return;
	}
	int ret = pwc->checkMD5(g_szFile, g_szData);

	if(ret)
		printf("%s check md5 error\n", g_szFile);
	else
		printf("%s md5 is the same\n", g_szFile);

}

static void chmodFile(WorkClient *pwc, int argc, char* argv[])
{
	int ret = 0;
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -M -s 10.10.10.10 -p 51116 [File] 
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -M -s 10.10.10.10 [File] 
			strcpy(g_szFile, argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	ret = pwc->chmodFile(g_szFile);

	if(ret)
		printf("%s chmod error\n", g_szFile);
	else
		printf("%s chmod success\n", g_szFile);

}

static void killProcess(WorkClient *pwc, int argc, char* argv[])
{
	int ret = 0;
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -K -s 10.10.10.10 -p 51116 [File] 
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -K -s 10.10.10.10 [File] 
			strcpy(g_szFile, argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	ret = pwc->killProcess(g_szFile);

	if(ret)
		printf("%s Kill error\n", g_szFile);
	else
		printf("%s Kill success\n", g_szFile);

}

static void execProcess(WorkClient *pwc, int argc, char* argv[])
{
	int ret = 0;
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -S -s 10.10.10.10 -p 51116 [File] 
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -S -s 10.10.10.10 [File] 
			strcpy(g_szFile, argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	ret = pwc->execProcess(g_szFile);

	if(ret)
		printf("%s exec error\n", g_szFile);
	else
		printf("%s exec success\n", g_szFile);

}

static void RPCShell(WorkClient *pwc, int argc, char* argv[])
{
	if(*g_szFile == 0)
	{
		if(g_bport && argc == 3)
		{
			//[program] -P -s 10.10.10.10 -p 51116 [shell] 
			strcpy(g_szFile, argv[2]);
		}
		else if(!g_bport && argc == 1)
		{
			//[program] -P -s 10.10.10.10 [File] 
			strcpy(g_szFile, argv[0]);
		}
	}

	if(*g_szFile == 0)
	{
		printf("File Name is NULL!\n");
		return;
	}

	string result;
    pwc->RPCShell(g_szFile, result);
	printf(result.c_str());

}

int main(int argc, char* argv[])
{
	
	if( argc == 1)
	{
		usage(argv[0]);
		return -1;
	}
	

	char szIp[20] = {0};
	int  iPort = 51116;
	int  iCommand = 0;

	const char* short_options="hAEDFLGCRMKSPs:p:f:l:r:d:";

	const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"Create", 0, NULL, 'A'},
		{"Exist", 0, NULL, 'E'},
		{"Delete", 0, NULL, 'D'},
		{"File", 0, NULL, 'F'},
		{"List", 0, NULL, 'L'},
		{"getmd5", 0, NULL, 'G'},
		{"Checksum", 0, NULL, 'C'},
		{"Rename", 0, NULL, 'R'},
		{"chMod", 0, NULL, 'M'},
		{"Kill", 0, NULL, 'K'},
		{"Start", 0, NULL, 'S'},
		{"RPC", 0, NULL, 'P'},
		{"ip", 1, NULL, 's'},
		{"port", 1, NULL, 'p'},
		{"file", 1, NULL, 'f'},
		{"list", 1, NULL, 'l'},
		{"root", 1, NULL, 'r'},
		{"data", 1, NULL, 'd'},
		{NULL, 0, NULL, 0},

	};

	int next = -1;

	while((next = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch(next)
		{
			/*
			 * command!
			 */
			//Help
			case 'h':
				usage(argv[0]);
				break;
			//Create A File
			case 'A':
				iCommand = 1;
				break;
			//File Exist
			case 'E':
				iCommand = 2;
				break;
		    //Delete A File
			case 'D':
				iCommand = 3;
				break;
			//Send A File
			case 'F':
				iCommand = 4;
				break;
			//Send A List
			case 'L':
				iCommand = 5;
				break;
			//Get File Md5
			case 'G':
				iCommand = 6;
				break;
			//Check File Md5
			case 'C':
				iCommand = 7;
				break;
			//Rename File
			case 'R':
				iCommand = 8;
				break;
			//Chmod File
			case 'M':
				iCommand = 9;
				break;
			//Kill Process
			case 'K':
				iCommand = 10;
				break;
			//Start Process
			case 'S':
				iCommand = 11;
				break;
			//RPC
			case 'P':
				iCommand = 12;
				break;


			/*
			 * Parameter
			 */
			case 's':
				strcpy(szIp, optarg);
				printf("ip: %s\n", szIp);
				break;
			case 'p':
				iPort = atoi(optarg);
				g_bport = true;
				printf("port:%d\n", iPort);
				break;
			case 'f':
				strcpy(g_szFile, optarg);
				printf("file:%s\n", g_szFile);
				break;
			case 'l':
				strcpy(g_szList, optarg);
				printf("list:%s\n", g_szList);
				break;
			case 'r':
				strcpy(g_szRoot, optarg);
				printf("root:%s\n", g_szRoot);
				break;
			case 'd':
				strcpy(g_szData, optarg);
				printf("data:%s\n", g_szData);
				break;
			default:
				usage(argv[0]);
				break;
		}
	}

	if(szIp[0] == 0)
	{
		return -1;
	}

	WorkClient wclient(szIp, iPort);
	
	switch(iCommand)
	{
		case 1:
			createFile(&wclient, argc - 4, argv + 4);
			break;
		case 2:
			checkFile(&wclient, argc - 4, argv + 4);
			break;
		case 3:
			removeFile(&wclient, argc - 4, argv + 4);
			break;
		case 4:
			sendFile(&wclient, argc - 4, argv + 4);
			break;
		case 5:
			sendList(&wclient, argc - 4, argv + 4);
			break;
		case 6:
			getMD5(&wclient, argc - 4, argv + 4);
			break;
		case 7:
			checkMD5(&wclient, argc - 4, argv + 4);
			break;
		case 8:
			renameFile(&wclient, argc - 4, argv + 4);
			break;
		case 9:
			chmodFile(&wclient, argc - 4, argv + 4);
			break;
		case 10:
			killProcess(&wclient, argc - 4, argv + 4);
			break;
		case 11:
			execProcess(&wclient, argc - 4, argv + 4);
			break;
		case 12:
			RPCShell(&wclient, argc - 4, argv + 4);
			break;
		default:
			break;
	}

	return 0;
}
