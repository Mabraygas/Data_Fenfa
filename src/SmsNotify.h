/*
 * SmsNotify.h
 *
 *  Created on: 2011-12-26
 *      Author:  wei.liu
 */

#ifndef SMSNOTIFY_H_
#define SMSNOTIFY_H_



/* ÿ��ѭ���ȴ������ļ�ʱ���ã�
 * 1�������� 1 Сʱ�����ļ��б��
 * 2�������� 33 Сʱ����ȫ�����£�
 */
int  OnNoInputFileLst();

/* 1��ÿ��ȫ������ʱ�������������ľ�����
 * 2����¼����ȫ�����µ�ʱ�䣻
 */
int  OnExportAll(const int iCntNLast, const int iCntN);

/* ÿ�촫��List�������ļ���Ŀ
 */
int  OnListSendPerDay(const int iCntList, const int iCntFile);

/* ���� $tmKick ��ʱ��ŷ��ͣ�
 */
int  SmsTimedSend(const time_t tmKick, const char * szMsg);

/* Sms �ӳٶ��е�ѭ�����
 */
int  SmsOnLoop(const time_t tmNow);

//���ű���
int SendSmsNotify(const char* msg);

#endif /* SMSNOTIFY_H_ */
