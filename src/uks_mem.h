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

/* �������ͷ� malloc �������ڴ�
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
	char		* pBuf_;	// ������
	int			iBufSize_;	// �������С
	int			iDatSize_;	// �����������ݴ�С
	int			iRIdx_;		// ������Ķ�ָ��ƫ��
	FILE 		* pfr_;		// ��ָ�루��ǰֻ��� getLine ʹ�ã�

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
		// ���ﲻ�ر� pfr_�����ⲿ���ùر�
	}

	// �Ƿ�ɶ��� $N �ֽ�����
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
			// ��� pout Ϊ NULL�����൱��ֱ���Թ� N �ֽ�
			memcpy(pout, pBuf_ + iRIdx_, N);
		}
		iRIdx_ += N;

		return  0;
	}

	/* ȡ��һ������� pout �У�һ�н����ı��Ϊ \r\n ���� \n��ÿ���� \n ��β��
	 * ���ʱ���� $pout ĩβ��� 0���Ҹ��ֽڰ����� $iLmt �����ڣ�
	 * return  < 0 : ���ݶ���
	 *         >=0 : �г��ȣ�������ĩβ 0��
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
			// ��󷵻ص��ַ������ȳ��� iBufSize_��������Ϊ iBufSize_ ��С
			iMaxStrLen = iBufSize_;
			// �� iMaxStrLen ȡ $iLmt-1 �� iBufSize_ �еĽ�Сֵ
		}
		int iLineEndIdx = iRIdx_;

		while (iLineEndIdx < iDatSize_ && '\n' != pBuf_[iLineEndIdx]) {
			int iLineLen = iLineEndIdx - iRIdx_ + 1; // ��ǰ��ɨ�賤��
			if (iLineLen >= iMaxStrLen) {
				// �� $iLmt ��Χ�ڻ�û�� \n����ʱ����ֱ�Ӹ������ݲ�����
				memcpy(pout, pBuf_ + iRIdx_, iMaxStrLen);
				*((char *)pout + iMaxStrLen) = 0;
				iRIdx_ += iMaxStrLen;
				return  iMaxStrLen;
			}
			iLineEndIdx ++;
		}
		if (iLineEndIdx < iDatSize_ && '\n' == pBuf_[iLineEndIdx]) {
			// �ҵ� '\n'
			int iStrLen = iLineEndIdx - iRIdx_ + 1;
			memcpy(pout, pBuf_ + iRIdx_, iStrLen);
			*((char *)pout + iStrLen) = 0;
			iRIdx_ += iStrLen;
			return  iStrLen;
		}

		// û���ҵ� '\n'������ $iLmt ��Χ�ڣ������ iDatSize_ ��������Ҫ�ٶ���һЩ����
		int ret = append(pfr_);
		if (ret < 0) {
			// �ļ����꣬iDatSize_ �������һ����
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

	/* �򻺳���������ݣ�
	 * �ᵼ����ִ��һ�ι������Ƚ�ʣ��������ǰ�ƶ���Ȼ��׷�������ݣ�
	 * N �ȿ��û����ʱ��׷��ʧ��
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

	/* ���ļ��ж�ȡ�����ܶ���ֽ�׷�ӵ������У�
	 * refer : append(const void * pin, const int N)
	 * return  -1 : �ļ�����
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

/* �����ļ�����Ļ���
 * ��ǰ��ʵ�ֱȽ�����
 */
struct  WStreamBuf {
	FILE 		* pfw_;

	char		* pBuf_;	// д����
	int			iBufSize_;	// д�����С
	int			iDatSize_;	// д���������ݴ�С
	int			iWIdx_;		// д�������д����
	uint64_t	ddwCurOff_;	// ��д�������+��ǰ���������ݳ��ȣ����൱�ڴ�д���¼���ļ��е�ƫ�ƣ�

	WStreamBuf(FILE * const pfw = NULL)
	: pfw_(pfw), pBuf_(0), iBufSize_(0), iDatSize_(0), iWIdx_(0), ddwCurOff_(0) {
	}

	~WStreamBuf() {
		// �����ڴ�ʹ�� flush����Ϊ��ʱ�ļ�ָ������Ѿ���Ч(�ļ�ָ�����ⲿ�ر�)
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

	/* �򻺳���������ݣ�
	 * ������ݿռ䲻��ᵼ����ִ��һ��д����Ȼ��ʣ��������ǰ�ƶ�����׷�������ݣ�
	 * N �ȿ��û����ʱ��ֱ��д�������̣����� fwrite ����ֵ��
	 * return    -1 :
	 *         < -1 : flush ʧ�ܣ�д�����ʧ�ܣ�
	 *            0 : OK
	 */
	int  bufWrite(const void * pin, const int N) {
		if (! pBuf_) {
			iBufSize_ = 40 << 20;	// 40M
			pBuf_ = (char *) malloc(iBufSize_); // ���
			if (! pBuf_) {
				iBufSize_ = 0; // �൱��û�л��棬����ֱ��ʹ�� fwrite д��
			}
			else {
				memset(pBuf_, 0, iBufSize_);
			}
		}
		if (N > iBufSize_) {
			// �޷����뻺�棬ֱ��д��
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

		// ׷�Ӵ��������
		memcpy(pBuf_ + iDatSize_, pin, N);
		iDatSize_ += N;
		ddwCurOff_ += N;

		return  0;
	}

	/* �������е�����ȫ��д��������
	 * refer : append(const void * pin, const int N)
	 * return  -1 : д���ļ�ʧ��
	 */
	int  flush() {
		if (! pfw_) {
			return  0;
		}
		int iLoop = 0; // ��ֹ������ѭ��
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

// ���ص���malloc��ʵ�ʷ���Ŀ����ڴ�Ĵ�С��
// size_t malloc_usable_size((void *__ptr));



#endif /* UKS_MEM_H_ */
