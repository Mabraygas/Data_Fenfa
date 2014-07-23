/*
 * uks_time.h
 *
 *  Created on: 2011-5-12
 *      Author:  wei.liu
 */

#ifndef UKS_TIME_H_
#define UKS_TIME_H_

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

// 返回以毫秒计的时间
uint64_t GetTimeMs();

// 返回 $tv2 - $tv1 的微秒值
uint64_t TimevalDiff(const struct timeval & tv1, const struct timeval & tv2);


int  GetTmDay(time_t tmSec = 0);


#endif /* UKS_TIME_H_ */
