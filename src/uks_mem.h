/*
 * uks_mem.h
 *
 *  Created on: 2011-5-13
 *      Author: wei.liu
 */

#ifndef UKS_MEM_H_
#define UKS_MEM_H_

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

char * DumpMem(const char * in, size_t len_prt, char * out, size_t len_out);

char * DumpStr(const char * in, const size_t iInLen = 0, char * out = NULL, size_t len_out = 0);

/* 仅用于释放 malloc 出来的内存
 */
template <typename T>
struct  LocalPtr {
	T	* p_;

	LocalPtr(T * p) {
		p_ = p;
	}

	~LocalPtr() {
		if (p_) {
			free(p_);
		}
		p_ = NULL;
	}
};

struct  RStreamBuf {
	char		* pBuf_;	// 读缓冲
	int			iBufSize_;	// 读缓冲大小
	int			iDatSize_;	// 读缓冲中数据大小
	int			iRIdx_;		// 读缓冲的读指针偏移
	FILE 		* pfr_;		// 读指针（当前只配合 getLine 使用）

	RStreamBuf(const int iBufSize = 0) : pBuf_(0), iBufSize_(iBufSize), iDatSize_(0), iRIdx_(0) {
		if (! pBuf_) {
			if (0 >= iBufSize_) {
				iBufSize_ = 40 << 20;	// 40M
			}
			pBuf_ = (char *) malloc(iBufSize_);
			//printf("RStreamBuf Malloc\n");
		}
	}

	~RStreamBuf() {
		if (pBuf_) {
			::free(pBuf_);
			//printf("RStreamBuf Free\n");
		}
		// 这里不关闭 pfr_，由外部调用关闭
	}

	// 是否可读出 $N 字节数据
	int  has(const int N) {
		if (iRIdx_ + N <= iDatSize_) {
			return  1;
		}
		return  0;
	}

	int  freeSize() {
		return  iBufSize_ - iDatSize_;
	}

	int  consume(void * pout, const int N) {
		if (iRIdx_ + N > iDatSize_) {
			return  -1;
		}
		if (pout) {
			// 如果 pout 为 NULL，则相当于直接略过 N 字节
			memcpy(pout, pBuf_ + iRIdx_, N);
		}
		iRIdx_ += N;

		return  0;
	}

	/* 取出一行输出到 pout 中，一行结束的标记为 \r\n 或者 \n，每行以 \n 结尾；
	 * 输出时会在 $pout 末尾添加 0，且该字节包含在 $iLmt 数量内；
	 * return  < 0 : 数据读完
	 *         >=0 : 行长度（不包含末尾 0）
	 */
	int  getLine(void * pout, const int iLmt) {
		if (iLmt < 2) {
			if (iLmt > 0) {
				*(char *)pout = 0;
			}
			return  0;
		}

		int iMaxStrLen = iLmt - 1;
		if (iMaxStrLen > iBufSize_) {
			// 最大返回的字符串长度超过 iBufSize_，则修正为 iBufSize_ 大小
			iMaxStrLen = iBufSize_;
			// 即 iMaxStrLen 取 $iLmt-1 和 iBufSize_ 中的较小值
		}
		int iLineEndIdx = iRIdx_;

		while (iLineEndIdx < iDatSize_ && '\n' != pBuf_[iLineEndIdx]) {
			int iLineLen = iLineEndIdx - iRIdx_ + 1; // 当前已扫描长度
			if (iLineLen >= iMaxStrLen) {
				// 在 $iLmt 范围内还没有 \n，此时可以直接复制数据并返回
				memcpy(pout, pBuf_ + iRIdx_, iMaxStrLen);
				*((char *)pout + iMaxStrLen) = 0;
				iRIdx_ += iMaxStrLen;
				return  iMaxStrLen;
			}
			iLineEndIdx ++;
		}
		if (iLineEndIdx < iDatSize_ && '\n' == pBuf_[iLineEndIdx]) {
			// 找到 '\n'
			int iStrLen = iLineEndIdx - iRIdx_ + 1;
			memcpy(pout, pBuf_ + iRIdx_, iStrLen);
			*((char *)pout + iStrLen) = 0;
			iRIdx_ += iStrLen;
			return  iStrLen;
		}

		// 没有找到 '\n'，且在 $iLmt 范围内，因此是 iDatSize_ 不够，需要再读入一些数据
		int ret = append(pfr_);
		if (ret < 0) {
			// 文件读完，iDatSize_ 就是最后一行了
			if (iDatSize_) {
				int iStrLen = iDatSize_;
				memcpy(pout, pBuf_ + iRIdx_, iStrLen);
				*((char *)pout + iStrLen) = 0;
				iRIdx_ += iStrLen;
				return  iStrLen;
			}
			else {
				return  -1;
			}
		}

		return  getLine(pout, iLmt);
	}

	/* 向缓冲中添加数据；
	 * 会导致先执行一次规整，先将剩余数据向前移动，然后追加新数据；
	 * N 比可用缓冲多时会追加失败
	 */
	int  append(const void * pin, const int N) {
		shrink();

		if (iDatSize_ + N > iBufSize_) {
			return  -1;
		}
		memcpy(pBuf_ + iDatSize_, pin, N);
		iDatSize_ += N;

		return  0;
	}

	/* 从文件中读取尽可能多的字节追加到缓冲中；
	 * refer : append(const void * pin, const int N)
	 * return  -1 : 文件结束
	 */
	int  append(FILE * pfr) {
		shrink();

		int iFree = iBufSize_ - iDatSize_;
		if (iFree) {
			int iRead = fread(pBuf_ + iDatSize_, 1, iFree, pfr);
			if (iRead < 1) {
				return  -1;
			}
			iDatSize_ += iRead;
		}

		return  0;
	}

	int  shrink() {
		memmove(pBuf_, pBuf_ + iRIdx_, iDatSize_ - iRIdx_);
		iDatSize_ -= iRIdx_;
		iRIdx_ = 0;

		return  0;
	}

	char * getWptr() {
		return  pBuf_ + iDatSize_;
	}
};

/* 用于文件输出的缓冲
 * 当前的实现比较蹩脚
 */
struct  WStreamBuf {
	FILE 		* pfw_;

	char		* pBuf_;	// 写缓冲
	int			iBufSize_;	// 写缓冲大小
	int			iDatSize_;	// 写缓冲中数据大小
	int			iWIdx_;		// 写缓冲的已写出量
	uint64_t	ddwCurOff_;	// 已写入磁盘量+当前缓存中数据长度（即相当于待写入记录在文件中的偏移）

	WStreamBuf(FILE * const pfw = NULL)
	: pfw_(pfw), pBuf_(0), iBufSize_(0), iDatSize_(0), iWIdx_(0), ddwCurOff_(0) {
	}

	~WStreamBuf() {
		// 不能在此使用 flush，因为此时文件指针可能已经无效(文件指针由外部关闭)
		if (pBuf_) {
			::free(pBuf_);
		}
	}

	int  clear() {
		pfw_ = NULL;
		iDatSize_ = 0;
		iWIdx_	  = 0;
		ddwCurOff_ = 0;
        return 0;
	}

	int  freeSize() {
		return  iBufSize_ - iDatSize_;
	}

	/* 向缓冲中添加数据；
	 * 如果数据空间不足会导致先执行一次写出，然后将剩余数据向前移动，再追加新数据；
	 * N 比可用缓冲大时将直接写出到磁盘，返回 fwrite 返回值；
	 * return    -1 :
	 *         < -1 : flush 失败（写入磁盘失败）
	 *            0 : OK
	 */
	int  bufWrite(const void * pin, const int N) {
		if (! pBuf_) {
			iBufSize_ = 40 << 20;	// 40M
			pBuf_ = (char *) malloc(iBufSize_); // 如果
			if (! pBuf_) {
				iBufSize_ = 0; // 相当于没有缓存，后面直接使用 fwrite 写出
			}
			else {
				memset(pBuf_, 0, iBufSize_);
			}
		}
		if (N > iBufSize_) {
			// 无法放入缓存，直接写出
			int ret = flush();
			if (ret < 0) {
				return  -1;
			}
			int iLoop = 0;
			int iWriten = 0;
			while (iWriten < N && iLoop < 10) {
				iLoop ++;
				ret = fwrite((const char *)pin + iWriten, 1, N - iWriten, pfw_);
				if (ret > 0) {
					iWriten += ret;
					ddwCurOff_ += ret;
				}
			}
			if (iLoop >= 10) {
				return  -2;
			}
			return ret;
		}
		if (iDatSize_ + N > iBufSize_) {
			int ret = flush();
			if (ret) {
				return  -10 + ret;
			}
		}

		// 追加待输出数据
		memcpy(pBuf_ + iDatSize_, pin, N);
		iDatSize_ += N;
		ddwCurOff_ += N;

		return  0;
	}

	/* 将缓存中的数据全部写到磁盘上
	 * refer : append(const void * pin, const int N)
	 * return  -1 : 写入文件失败
	 */
	int  flush() {
		if (! pfw_) {
			return  0;
		}
		int iLoop = 0; // 防止出现死循环
		int iDatLen = iDatSize_ - iWIdx_;
		while (iDatLen > 0 && iLoop < 10) {
			iLoop ++;
			int iWrite = fwrite(pBuf_ + iWIdx_, 1, iDatLen, pfw_);
			if (iWrite < 0) {
				usleep(1000);
				continue;
			}
			iWIdx_ += iWrite;
			iDatLen = iDatSize_ - iWIdx_;
		}
		if (iLoop >= 10) {
			return  -1;
		}
		shrink();

		return  0;
	}

	int  shrink() {
		memmove(pBuf_, pBuf_ + iWIdx_, iDatSize_ - iWIdx_);
		iDatSize_ -= iWIdx_;
		iWIdx_ = 0;

		return  0;
	}

};

// 返回调用malloc后实际分配的可用内存的大小。
// size_t malloc_usable_size((void *__ptr));



#endif /* UKS_MEM_H_ */
