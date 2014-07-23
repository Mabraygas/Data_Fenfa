#include "data_verify.h"
#include "DayLog.h"

#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)
DayLog g_log;

int main(int argc, char** argv)
{
	if(strcmp("-h", argv[1]) == 0)
	{
		printf("usage:%s [xml File]\n", argv[0]);
		return -1;
	}

	DataVerify dv;
	if(argc > 2)
		dv.InitSys(argv[1]);
	else
		dv.InitSys(NULL);

	dv.start();
	dv.join();
	return 0;
}
