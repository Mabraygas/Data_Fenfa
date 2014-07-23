/*
 * SmsNotify.h
 *
 *  Created on: 2011-12-26
 *      Author:  wei.liu
 */

#ifndef SMSNOTIFY_H_
#define SMSNOTIFY_H_



/* 每次循环等待输入文件时调用；
 * 1、报警于 1 小时内无文件列表到达；
 * 2、报警于 33 小时内无全量更新；
 */
int  OnNoInputFileLst();

/* 1、每次全量更新时，进行数据量的警报；
 * 2、记录本次全量更新的时间；
 */
int  OnExportAll(const int iCntNLast, const int iCntN);

/* 每天传输List数量和文件数目
 */
int  OnListSendPerDay(const int iCntList, const int iCntFile);

/* 到达 $tmKick 的时候才发送；
 */
int  SmsTimedSend(const time_t tmKick, const char * szMsg);

/* Sms 延迟队列的循环检查
 */
int  SmsOnLoop(const time_t tmNow);

//短信报警
int SendSmsNotify(const char* msg);

#endif /* SMSNOTIFY_H_ */
