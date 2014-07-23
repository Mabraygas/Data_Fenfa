/*
 * uks_str.h
 *
 *  Created on: 2011-5-9
 *      Author: wei.liu
 */

#ifndef UKS_STR_H_
#define UKS_STR_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>

/* ÔÚ $szBuf µÄ»ù´¡ÉÏ½Ø¶ÏÓÒ²à¿Õ°××Ö·û ( \t\r\n)
 * return  Ê£ÏÂ×Ö·û´®µÄ strlen
 */
int  LocalRTrim(char * szBuf) {
	long idx = strlen(szBuf) - 1;
	if (idx < 0) {
		return  0;
	}
	// 0x5c  92  \
	// 0x2f  47  /
	//
	while (szBuf[idx] == ' ' || szBuf[idx] == '\n'
			|| szBuf[idx] == '\r' || szBuf[idx] == '\t') {
		idx --;
		if (idx < 0) {
			szBuf[0] = 0;
			return  0;
		}
	}
	szBuf[idx + 1] = 0;

	return  idx + 1;
}

/* ÔÚ $szBuf µÄ»ù´¡ÉÏÉ¾³ý×ó²à¿Õ°××Ö·û£¬½«Ê£Óà×Ö·ûÒÆ¶¯µ½ $szBuf[0] ´¦
 */
int  LocalLTrim(char * szBuf) {
	long lLen = strlen(szBuf);
	if (lLen < 1) {
		return  0;
	}
	long lr = 0;
	while (szBuf[lr] == ' ' || szBuf[lr] == '\n' || szBuf[lr] == '\r'
			|| szBuf[lr] == '\t') {
		szBuf[lr] = 0;
		lr ++;
	}
	if (lr != 0) {
		int iLeftLen = lLen - lr;
		memmove(szBuf, szBuf + lr, iLeftLen);
		szBuf[iLeftLen] = 0;
	}

	return  lLen - lr;
}


#endif /* UKS_STR_H_ */
