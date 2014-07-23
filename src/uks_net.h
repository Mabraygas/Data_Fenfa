/*
 * uks_net.h
 *
 *  Created on: 2011-5-10
 *      Author: wei.liu
 */

#ifndef UKS_NET_H_
#define UKS_NET_H_


#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include "Eps_WIN32_LINUX.h"


// 设置 $pstSockAddr
int  set_sockaddr(struct sockaddr_in * pstSockAddr, const char * ip, int port);
int  MakeSockaddr(struct sockaddr_in * pstSockAddr, const char * ip, int port);

// 设置非阻塞
int  SockSetSRTmout(int sock, int iTmoutUs, char * pErrMsg = NULL, int iEMsgLmt = 0);
int  SockSetNoDelay(int sock, int iNodelay, char * pErrMsg = NULL, int iEMsgLmt = 0);
int  SockSetNblk   (int sock, char * pErrMsg = NULL, int iEMsgLmt = 0);
int  SockSetBlk    (int sock, char * pErrMsg = NULL, int iEMsgLmt = 0);

// Tcp 部分
// 创建 listen socket, 并 listen
int  CreateServerSock(const char * szIp, const int iPort, char * pErrMsg = NULL, const int iEMsgLmt = 0);


// Connect
int  NewTcpBlkConn (int * psock, const char *IpStr, unsigned short Port,
		char * pErrMsg, int iEMsgLmt);
int  NewTcpNblkConn(int * psock, const char *IpStr, unsigned short Port, int iTmoutUs,
		char * pErrMsg, int iEMsgLmt);

// Send
int  TcpBlkSendN (const int sock, char *Buf, long lBufLen, long *plSend,
		int iTmoutUs, char * pErrMsg = NULL, int iEMsgLmt = 0);
// deprecated
int  TcpBlkSend  (int sock, const char *Buf, const long lBufLen, long *plSend, char * pErrMsg, int iEMsgLmt);
// deprecated
int  TcpNblkSend (int sock, const char *Buf, const long lBufLen, long *plSend, int iTmoutUs, char * pErrMsg, int iEMsgLmt);
// deprecated
int  TcpTmoutSend(int sock, const char *Buf, const long lBufLen, long *plSend, int iTmoutUs, char * pErrMsg, int iEMsgLmt);


/* 可用于阻塞/非阻塞 socket 的带超时的接收
在不超过大约 $iTmoutUs 微秒的时间内接收 $lExpectLen 多的数据
*$plRecv 返回接收数据的长度
return  >0 : 没有错误
 0 && errno==ETIME : 远端关闭连接
        -1 : select 错误
      <-100: recv 错误
*/
int  TcpBlkRecvN (const int sock, char *Buf, long lExpectLen, long *plRecv,
		int iTmoutUs, char * pErrMsg = NULL, int iEMsgLmt = 0);
// deprecated
int  TcpBlkRecv  (int sock, char *Buf, const long lExpectLen, long *plRecv, int iTmoutUs, char * pErrMsg, int iEMsgLmt);
// deprecated
int  TcpNblkRecv (int sock, char *Buf, const long lExpectLen, long *plRecv, int iTmoutUs, char * pErrMsg, int iEMsgLmt);
// deprecated
int  TcpTmoutRecv(int sock, char *Buf, const long lExpectLen, long *plRecv, int iTmoutUs, char * pErrMsg, int iEMsgLmt);

/* ********************  UDP  ********************
 *
 * *********************************************** */


/* 创建一个 UDP socket，其绑定地址为 $szAddr:$port
 * return >= 0 : sock
 *         < 0 : error
 */
int CreateUDPSocket(const char * szAddr, const unsigned short wPor, char * pErrMsg = NULL, int iEMsgLmt = 0);

int CreateUDPSocket(char * pErrMsg = NULL, int iEMsgLmt = 0);


/* ******************  INet 层  ****************** */

int  GetIntfaceIp(const char *interface_name, char * ip);
int  GetSockBufLen(int iSock, int * piRecvBufLen, int * piSendBufLen);
int  SetSockBufLen(int iSock, int iRecvBufLen, int iSendBufLen);

/* 遍历各个网络接口
 */
int ListAllInterface();



struct  EpollMng {
	static const int	FLAG_READ  = 0x1;
	static const int	FLAG_WRITE = 0x2;
	static const int	FLAG_ERROR = 0x4;
	static const int	FLAG_RW    = FLAG_READ | FLAG_WRITE;
	static const int	FLAG_RE    = FLAG_READ | FLAG_ERROR;
	static const int	FLAG_ALL   = FLAG_READ | FLAG_WRITE | FLAG_ERROR;

	int		iEpFd_;
	int		iMaxEvents_;
	struct epoll_event	* pstEvts;

	CRITICAL_SECTION 		mutex_;


	EpollMng(int iMaxEvents = 1024) : iEpFd_(-1), iMaxEvents_(iMaxEvents),
			pstEvts(NULL) {
		InitializeCriticalSection(&mutex_);
	}

	~EpollMng() {
		DeleteCriticalSection(&mutex_);
	}

	int  Init(int iMaxEvents = 1024) {
		if (iMaxEvents < 1) {
			return  -1;
		}
		if (iEpFd_ > 0) {
			close(iEpFd_);
		}
		if (pstEvts) {
			free(pstEvts);
		}
		iMaxEvents_ = iMaxEvents;
		pstEvts = (struct epoll_event *)malloc(sizeof(*pstEvts) * iMaxEvents_);
		memset(pstEvts, 0, sizeof(*pstEvts) * iMaxEvents_);

		int ret = epoll_create(iMaxEvents_); // TODO
		if (ret < 0) {
			return  -2;
		}
		iEpFd_ = ret;

		return  0;
	}

	int  AddFd(const int sock, const int iFlag,
			char * pErrMsg = NULL, const int iEMsgLmt = 0);

	int  ModFd(const int sock, const int iFlag,
			char * pErrMsg = NULL, const int iEMsgLmt = 0);

	int  DelFd(const int sock,
			char * pErrMsg = NULL, const int iEMsgLmt = 0);

	// $iMSec 等待的毫秒数
	int  Wait(int iMSec, int * piNFds) {
		errno = 0;
		int nfds = epoll_wait(iEpFd_, pstEvts, iMaxEvents_, iMSec);
		if (nfds < 0) {
			return  -1;
		}
		* piNFds = nfds;
		return  0;
	}

	int  CanRead(int iIdx) {
		if (EPOLLIN & pstEvts[iIdx].events) {
			return  1;
		}
		return  0;
	}

	int  CanWrite(int iIdx) {
		if (EPOLLOUT & pstEvts[iIdx].events) {
			return  1;
		}
		return  0;
	}

	int  HasError(int iIdx) {
		if ((EPOLLHUP | EPOLLPRI | EPOLLERR) & pstEvts[iIdx].events) {
			return  1;
		}
		return  0;
	}
};

struct  IpAddr {
	char			szIp_[20];
	unsigned short	wPort_;
};

#endif /* UKS_NET_H_ */
