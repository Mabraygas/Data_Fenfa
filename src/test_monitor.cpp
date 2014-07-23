#include "symbol_monitor.h"
#include "DayLog.h"

#define  LOG(format, args...) g_log.TWrite("%s:%s(%d) " format, __FILE__, __FUNCTION__, __LINE__, ##args)
DayLog g_log;

int main(int argc, char** argv)
{
	SymbolMonitor sm;
	sm.Init(NULL, NULL);
	sm.start();
	sm.join();
	return 0;
}
