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

// ����ͳ����� 10 �ε��������ݴ����������һֱΪ 0�������������ݵĸ�ʽ���ܲ���
// �仯

static time_t	g_tmLastExportAll = 0; // �ϴ�ȫ�����µ�ʱ��

/* ���� û���ļ������б� ʱ��֪ͨ */
static int 		g_iNoFLstHours = 0; // �����ۼ�Сʱ��
static time_t	g_tmLastInputFileList = 0; // ���һ�η��͵�ʱ��

static TimedQueue	g_delayMsgQue;

// TODO ���ڷ�ֹ����ʱ�����������
static int 		g_iCntSmsSend = 0; // ÿ����ŷ�����

//List ���ļ��ۼ���
static int      g_iCntList = 0;    //ÿ��List����
static int      g_iCntFile = 0;    //ÿ�촦����ļ�����
static time_t   g_tmLastSend = 0;

static int  SmsNotify(const char * szMsg) {
	const char * szPhoneNums = "18911059121";//
	const char * szIp = "10.103.11.62";
	const uint16_t wPort = 20229;
	//int ret = 0;

	static int		iToday = 0; // �죨���ڣ�
	const int day = GetTmDay();
	if (iToday != day) {
		// ��һ�����������
		g_iCntSmsSend = 0;
		iToday = day;
	}

	// ��ֹ����Ƶ����֪ͨ
	//int  iSend = 0; // ��Ƕ����Ƿ���������ȥ
	static time_t  tmLastSend = 0;
	if (time(NULL) > tmLastSend + 10 && g_iCntSmsSend < 30) { // �ۼƶ��ŷ������������ڷ�ֹ���෢��
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

/* ÿ��ѭ���ȴ������ļ�ʱ���ã�
 * 1�������� 1 Сʱ�����ļ��б��
 * 2�������� 33 Сʱ����ȫ�����£�
 */
int  OnNoInputFileLst() {
	time_t  tmNow = time(NULL);

	{
		if (g_tmLastInputFileList && tmNow > g_tmLastInputFileList + 3600) {
			// tmLastInputFileList �Ѿ���ʼ����
			// ���Ҿ����ϴγ����ļ��б��Ѿ��� 1 Сʱ����������ļ�����
			g_iNoFLstHours ++;
			char szMsg[300];
			snprintf(szMsg, sizeof(szMsg), "No sync file list come for %d hours", g_iNoFLstHours);
			SmsNotify(szMsg);
			g_tmLastInputFileList = tmNow; // �� 1 Сʱ�ٱ�һ��
		}
	}

	{	// ���� 33 Сʱû��ȫ�������򱨾�
		if (g_tmLastExportAll && tmNow > g_tmLastExportAll + 33*3600) {
			// ������� 33 Сʱû�н���ȫ�����£�����Ҫ����
			char szMsg[300];
			snprintf(szMsg, sizeof(szMsg), "No ExportAll for %d hours", (int)(tmNow - g_tmLastExportAll)/3600);
			SmsNotify(szMsg);
			g_tmLastExportAll = tmNow - 30 * 3600;// �� 3 Сʱ�ٱ�һ��
		}
	}

	return  0;
}


/* 1��ÿ��ȫ������ʱ�������������ľ�����
 * 2����¼����ȫ�����µ�ʱ�䣻
 */
int  OnExportAll(const int iCntNLast, const int iCntN) {
	// 1��ÿ��ȫ������ʱ�������������ľ�����
	if (iCntNLast > iCntN) {
		char szMsg[300];
		snprintf(szMsg, sizeof(szMsg), "warn:ExportAll %d -> %d", iCntNLast, iCntN);
		SmsNotify(szMsg);
	}

	// 2����¼����ȫ�����µ�ʱ��
	g_tmLastExportAll = time(NULL);

	// 3�����Է���ÿ���������
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
/* ���� $tmFuture ��ʱ��ŷ��ͣ�
 */
int  SmsTimedSend(const time_t tmKick, const char * szMsg) {

	int ret = g_delayMsgQue.push(tmKick, szMsg);

	return  ret;
}

/* Sms �ӳٶ��е�ѭ�����
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

