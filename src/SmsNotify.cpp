/*
 * SmsNotify.cpp
 *
 *  Changed on: 2012-10-29
 *      Author:  Yuantu
 */
#include  <time.h>
#include  "SmsNotify.h"
#include  "uks_sms.h"
//#include  "uks_log.h"
//#include  "uks_micro.h"
#include  "uks_time.h"
//#include  "uks_net.h"


// ---------------- extern ----------------

//extern  CDayLog  g_log;
//extern  CDayLog  g_warnlog;
//extern  CDayLog  g_errlog;
//extern  CDayLog   g_Srvlog;
extern  int		 g_iSendSms;

// ---------------- static ----------------

// 用于统计最近 10 次到来的数据处理量，如果一直为 0，表明输入数据的格式可能产生
// 变化

static time_t	g_tmLastExportAll = 0; // 上次全量更新的时间

/* 用于 没有文件输入列表 时的通知 */
static int 		g_iNoFLstHours = 0; // 连续累计小时数
static time_t	g_tmLastInputFileList = 0; // 最近一次发送的时间

static TimedQueue	g_delayMsgQue;

// TODO 用于防止测试时产生过多短信
static int 		g_iCntSmsSend = 0; // 每天短信发送量

//List 和文件累计量
static int      g_iCntList = 0;    //每天List数量
static int      g_iCntFile = 0;    //每天处理的文件数量
static time_t   g_tmLastSend = 0;

static int  SmsNotify(const char * szMsg) {
	const char * szPhoneNums = "18911059121";//
	const char * szIp = "10.103.11.62";
	const uint16_t wPort = 20229;
	//int ret = 0;

	static int		iToday = 0; // 天（日期）
	const int day = GetTmDay();
	if (iToday != day) {
		// 过一天后发送量清零
		g_iCntSmsSend = 0;
		iToday = day;
	}

	// 防止过分频繁地通知
	//int  iSend = 0; // 标记短信是否真正发出去
	static time_t  tmLastSend = 0;
	if (time(NULL) > tmLastSend + 10 && g_iCntSmsSend < 30) { // 累计短信发送条数，用于防止过多发送
		//if (g_iSendSms) {
			SendSms(szIp, wPort, szPhoneNums, szMsg);
		//}
		tmLastSend = time(NULL);
		//iSend = 1;
	}
	g_iCntSmsSend++;

	//LOG(&g_log, "<SMS> [%d,%d] sendto %s:%d '%s'\n", g_iCntSmsSend, iSend, szIp, (int)wPort, szMsg);

	return  0;
}

/* 每次循环等待输入文件时调用；
 * 1、报警于 1 小时内无文件列表到达；
 * 2、报警于 33 小时内无全量更新；
 */
int  OnNoInputFileLst() {
	time_t  tmNow = time(NULL);

	{
		if (g_tmLastInputFileList && tmNow > g_tmLastInputFileList + 3600) {
			// tmLastInputFileList 已经初始化过
			// 并且距离上次出现文件列表已经有 1 小时，则产生无文件报警
			g_iNoFLstHours ++;
			char szMsg[300];
			snprintf(szMsg, sizeof(szMsg), "No sync file list come for %d hours", g_iNoFLstHours);
			SmsNotify(szMsg);
			g_tmLastInputFileList = tmNow; // 过 1 小时再报一次
		}
	}

	{	// 超过 33 小时没有全量更新则报警
		if (g_tmLastExportAll && tmNow > g_tmLastExportAll + 33*3600) {
			// 如果超过 33 小时没有进行全量更新，则需要报警
			char szMsg[300];
			snprintf(szMsg, sizeof(szMsg), "No ExportAll for %d hours", (int)(tmNow - g_tmLastExportAll)/3600);
			SmsNotify(szMsg);
			g_tmLastExportAll = tmNow - 30 * 3600;// 过 3 小时再报一次
		}
	}

	return  0;
}


/* 1、每次全量更新时，进行数据量的警报；
 * 2、记录本次全量更新的时间；
 */
int  OnExportAll(const int iCntNLast, const int iCntN) {
	// 1、每次全量更新时，进行数据量的警报；
	if (iCntNLast > iCntN) {
		char szMsg[300];
		snprintf(szMsg, sizeof(szMsg), "warn:ExportAll %d -> %d", iCntNLast, iCntN);
		SmsNotify(szMsg);
	}

	// 2、记录本次全量更新的时间
	g_tmLastExportAll = time(NULL);

	// 3、测试发条每日输出短信
	{
		char  szMsg[300];
		snprintf(szMsg, sizeof(szMsg), "info:ExportAll %d -> %d", iCntNLast, iCntN);
		SmsTimedSend(time(NULL) + 9*3600, szMsg);
	}
	return  0;
}

int SendSmsNotify(const char* msg)
{
	SmsNotify(msg);
	return 0;
}

int  OnListSendPerDay(const int iCntList, const int iCntFile)
{
	time_t tmNow = time(NULL);

	g_iCntList += iCntList;
	g_iCntFile += iCntFile;

	if(g_tmLastSend == 0)
	{
		g_tmLastSend = tmNow;
	}

	if(g_tmLastSend && tmNow > g_tmLastSend + 24*3600) 
	{
		char szMsg[300];
		snprintf(szMsg, sizeof(szMsg), "[Info]Last 24 hours send %d Lists and %d Files", g_iCntList, g_iCntFile);
		SmsNotify(szMsg);
		g_tmLastSend = tmNow; 
		g_iCntList   = 0;
		g_iCntFile   = 0;
	}

	return 0;
}
/* 到达 $tmFuture 的时候才发送；
 */
int  SmsTimedSend(const time_t tmKick, const char * szMsg) {

	int ret = g_delayMsgQue.push(tmKick, szMsg);

	return  ret;
}

/* Sms 延迟队列的循环检查
 */
int  SmsOnLoop(const time_t tmNow) {
	char  szMsg[500];
	szMsg[0] = 0;
	int ret = g_delayMsgQue.pop(szMsg, sizeof(szMsg), tmNow);
	if (ret < 0) {
		return  ret;
	}
	SmsNotify(szMsg);

	return  0;
}

