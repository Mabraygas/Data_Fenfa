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


/* 通过 $szIp:$port 短信服务器发送短信；
 * $szPhoneNums : 目的手机号码，各个号码以逗号 ',' 分隔；
 * $szMsg       : 短信内容；
 * return    0  : OK
 */
int  SendSms(const char * szIp, const uint16_t port,
		const char * szPhoneNums, const char * szMsg);

/* 通过 $szIp:$port 短信服务器发送短信；
 * $iCntPhone : 目的手机号码个数；
 * $apsPhone  : 目的手机号码；
 * $szMsg     : 短信内容；
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

	/* $tmKick : 消息可 pop 出的时间
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
			// 队列满
			return  -2;
		}
		// 此时 i 指向的是最后一个非 0 单元
		while (i >= 0 && tmKick < pstMsgs_[i].tmKick_) {
			// 将 $tmKick 时间之后的数据往后移
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
			// 无数据
			return  -1;
		}

		if (pstMsgs_[0].tmKick_ <= tmNow) {
			snprintf(szMsg, iMsgSize, "%s", pstMsgs_[0].szMsg_);
		}
		else {
			// 没有数据触发
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
	TimedMessage	* pstMsgs_; // 包含的数据按照 tmKick_ 由小到大排序
	int				iMaxMsg_;
};



#endif /* UKS_SMS_H_ */
