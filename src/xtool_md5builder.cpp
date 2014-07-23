/**
 * @file xtool_mdbuilder.cpp
 * @brief md5生成
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-08
 */

#include <iostream>
#include <sys/stat.h>

#include "md5.h"

#define  MAX_FILE_SIZE 6<<20
#define  MAX_SEND_SIZE 1<<20

using namespace std;

string FileDigest(const string& file) {

	struct stat stbuf;
	stat(file.c_str(), &stbuf);
	MD5 md5;
	string strmd5;

	if(stbuf.st_size > MAX_FILE_SIZE)
	{
		FILE* fpr = fopen(file.c_str(), "rb");
		if(!fpr)
		{
			printf("can not open %s\n", file.c_str());
			return "";
		}

		char* pBuf = new char[MAX_SEND_SIZE];

		int iDataSize = fread(pBuf, 1, MAX_SEND_SIZE, fpr);
		
		string strFile = string(pBuf, iDataSize);
		strFile.append((const char*)&stbuf.st_size, 8);
		md5.update(strFile);
		strmd5 = md5.toString();
		fclose(fpr);
	}
	else
	{
		ifstream in(file.c_str(), ios::binary);
		if(!in.is_open())
		{
			printf("can not open %s\n", file.c_str());
			return "";
		}

		md5.update(in);
		strmd5 = md5.toString();
		in.close();
	}
	return strmd5;
}

int main(int argc, char** argv) {

	if(argc!=2)
	{
		printf("usage:\n%s [File Name]\n", argv[0]);
		return -1;
	}

	char szMD5[300];
	sprintf(szMD5, "%s.md5", argv[1]);
	FILE* fpw = fopen(szMD5, "wb");
	if(!fpw)
	{
		printf("open file %s error!\n", szMD5);
		return -2;
	}

	string strmd5 = FileDigest(argv[1]);
	fprintf(fpw, "%s", strmd5.c_str());
	printf("'%s' md5:%s, md5 file:'%s'\n", argv[1], strmd5.c_str(), szMD5);
	fclose(fpw);
	return 0;
}
