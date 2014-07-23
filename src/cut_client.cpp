#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <eagle_thread.h>
#include <eagle_clientsocket.h>

#include "cut_client.h"

using namespace std;
using namespace eagle;


int  CutClient::MakeReqPkg(string & str, char * pkg) {
	// YKSEGEMSYKEmseg0 + 00000020 + 一个空格 + 优酷土豆集团 + YKEmseg0
	//char  len  = 0;
	char * pw = pkg;
	pw += sprintf(pw, "YKSEGEMSYKEmseg0");
	pw += sprintf(pw, "%.8d", str.length() + 8);
	*pw++ = ' ';
	memcpy(pw, str.c_str(), str.length());
	pw += str.length();
	pw += sprintf(pw, "YKEmseg0");
	return  pw - pkg;
}


int CutClient::getCutRes(string &str, string &result, string &err) {
		
	//TCPClient cs_(ip, port, g_timeout);
	//struct timeval  tmvS, tmvE;
	//GetTimeofDay(&tmvS);

	//发送数据
	char  req[4096] = {0};
	int pkgLen = MakeReqPkg(str, req);
	int ret = cs_.send(req, pkgLen, err);

	if (ret < 0) {
		cerr << " send failed (" << err << ")" << endl;
		assert(false);
	}

	//接收数据
	char  resp[2048] = {0};
	size_t	headLen = 17;
	ret = cs_.recv(resp, headLen, err);
	if (ret < 0) {
		cerr << " recv failed (" << err << ")" << endl;
		assert(false);
	}

	size_t bodyLen = atoi(resp + 8);
	if (bodyLen > sizeof(resp) - headLen) {
		cerr << "Bad body len " << bodyLen << endl;
		return -1;
	}
	ret = cs_.recv(resp + headLen, bodyLen, err);
	//GetTimeofDay(&tmvE);
	if (ret < 0) {
		cerr << " recv failed : " << err << endl;
		assert(false);
	}

	string body = string(resp + headLen);
	//printf("body:'%s'\n", body.c_str());
	int iResLen = 0;//body.length() - str.length() - 12;
	int iStrBegin = body.find("\r\n", 0) + 2;

	iResLen = body.length() - iStrBegin - 10;
	//printf("bodyLen: %d, reslen:%d\n", bodyLen, iResLen);
	
	//include common and combine cut result
	result = body.substr(iStrBegin, iResLen);
	//cout << id_ << "Info " << *it << " <-> " << resp + headLen << endl;

	//printf("Cut '%s' -> \n%s\n", str.c_str(), resp);
	//printf("----------------------------------------------\n");
	//printf("'%s' ::\n'%s'\n\n", str.c_str(), result.c_str());

	return 0;

}

int CutClient::getCutRes(string &str, string &common, string &combine, string &err) {
		
	//发送数据
	char  req[4096] = {0};
	int pkgLen = MakeReqPkg(str, req);
	int ret = cs_.send(req, pkgLen, err);

	if (ret < 0) {
		cerr << " send failed (" << err << ")" << endl;
		assert(false);
	}

	//接收数据
	char  resp[2048] = {0};
	size_t	headLen = 17;
	ret = cs_.recv(resp, headLen, err);
	if (ret < 0) {
		cerr << " recv failed (" << err << ")" << endl;
		assert(false);
	}

	size_t bodyLen = atoi(resp + 8);
	if (bodyLen > sizeof(resp) - headLen) {
		cerr << "Bad body len " << bodyLen << endl;
		return -1;
	}
	ret = cs_.recv(resp + headLen, bodyLen, err);
	//GetTimeofDay(&tmvE);
	if (ret < 0) {
		cerr << " recv failed : " << err << endl;
		assert(false);
	}

	char *p = resp +headLen;
	char *pre = p;

	char *ar[3] = {0};
	int i = 0;
	while(*p != '\0' && i < 3)
	{
		while(*p != '\r')
			p++;
		ar[i] = pre;

		if(*p == '\0')
			break;
		*p = 0;
		p+=2;
		i++;
		pre = p;
	}

	common = ar[1];
	combine = ar[2];

	/*
	for(i = 0; i < 3; i++)
	{
		printf("'%s'\n", ar[i]);
	}
	*/
	/*
	string body = string(resp + headLen);

	int iResLen = 0;
	int iStrBegin = body.find("\r\n", 0) + 2;

	iResLen = body.length() - iStrBegin - 10;
	
	string result;
	result = body.substr(iStrBegin, iResLen);

	int iCom = result.find("\r\n");
	common = result.substr(0, iCom);
	combine = result.substr(iCom+2, result.length() - iCom - 4);
	*/

	//printf("'%s' ::\n'%s'\n\n", str.c_str(), result.c_str());

	return 0;

}

