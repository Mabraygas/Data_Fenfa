/*
 * uks_sms.h
 *
 *  Created on: 2011-12-26
 *      Author:  wei.liu
 */

#ifndef UKS_SMS_H_
#define UKS_SMS_H_

#include  "Eps_WIN32_LINUX.h"
#include  "uks_net.h"


/* ͨ�� $szIp:$port ���ŷ��������Ͷ��ţ�
 * $szPhoneNums : Ŀ���ֻ����룬���������Զ��� ',' �ָ���
 * $szMsg       : �������ݣ�
 * return    0  : OK
 */
int  SendSms(const char * szIp, const uint16_t port,
		const char * szPhoneNums, const char * szMsg);

/* ͨ�� $szIp:$port ���ŷ��������Ͷ��ţ�
 * $iCntPhone : Ŀ���ֻ����������
 * $apsPhone  : Ŀ���ֻ����룻
 * $szMsg     : �������ݣ�
 * return  0 : OK
 */
int  SendSms(const char * szIp, const uint16_t port,
		const char * apsPhone[], const int iCntPhone, const char * szMsg);





struct  TimedMessage {
	time_t		tmKick_;
	char		szMsg_[500];

	TimedMessage() : tmKick_(0) {
		memset(szMsg_, 0, sizeof(szMsg_));
	}
};

struct  TimedQueue {


	TimedQueue(const int iMaxMsg = 20) : iMaxMsg_(iMaxMsg), pstMsgs_(NULL) {
		pstMsgs_ = new TimedMessage[iMaxMsg_];
	}

	~TimedQueue() {
		if (pstMsgs_) {
			delete [] pstMsgs_;
			pstMsgs_ = NULL;
		}
	}

	/* $tmKick : ��Ϣ�� pop ����ʱ��
	 */
	int  push(const time_t tmKick, const char * szMsg) {
		if (tmKick < 1) {
			return  -1;
		}

		int  i = 0;
		for (i = iMaxMsg_ - 1; i >= 0; i--) {
			if (pstMsgs_[i].tmKick_) {
				break;
			}
		}
		if (i == iMaxMsg_ - 1) {
			// ������
			return  -2;
		}
		// ��ʱ i ָ��������һ���� 0 ��Ԫ
		while (i >= 0 && tmKick < pstMsgs_[i].tmKick_) {
			// �� $tmKick ʱ��֮�������������
			pstMsgs_[i + 1] = pstMsgs_[i];
			i --;
		}
		i++;

		pstMsgs_[i].tmKick_ = tmKick;
		snprintf(pstMsgs_[i].szMsg_, sizeof(pstMsgs_[i].szMsg_), "%s", szMsg);

		return  0;
	}

	int  pop(char * szMsg, const int iMsgSize, time_t tmNow = 0) {
		if (tmNow < 1) {
			tmNow = time(NULL);
		}

		if (0 == pstMsgs_[0].tmKick_) {
			// ������
			return  -1;
		}

		if (pstMsgs_[0].tmKick_ <= tmNow) {
			snprintf(szMsg, iMsgSize, "%s", pstMsgs_[0].szMsg_);
		}
		else {
			// û�����ݴ���
			return  -2;
		}

		int  i = 0;
		for (i = 0; i < iMaxMsg_ - 1; i++) {
			pstMsgs_[i] = pstMsgs_[i + 1];
		}
		pstMsgs_[i].tmKick_   = 0;
		pstMsgs_[i].szMsg_[0] = 0;

		return  0;
	}

	int  size() {
		int i = 0;
		for (i = iMaxMsg_ - 1; i >= 0; i--) {
			if (pstMsgs_[i].tmKick_) {
				break;
			}
		}
		return  i + 1;
	}

private:
	TimedMessage	* pstMsgs_; // ���������ݰ��� tmKick_ ��С��������
	int				iMaxMsg_;
};



#endif /* UKS_SMS_H_ */
