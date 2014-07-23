#include <iostream>

#include "QSEPSDF_CS.h"
#include "md5.h"
#include "protocol.h"

using namespace std;

string FileDigest(const string& file) {

	ifstream in(file.c_str(), ios::binary);
	if (!in) {
		return "";
	}

	MD5 md5;
	std::streamsize length;
	char buffer[1024];
	while (!in.eof()) {
		in.read(buffer, 1024);
		length = in.gcount();
		if (length > 0) {
			md5.update(buffer, length);
		}
	}
	in.close();
	return md5.toString();
}

int main() {

	TCPClient* tcp = new TCPClient(string("127.0.0.1"), 51116);

	cout << "test_server:"<<FileDigest("test_server") << endl;

	string strname = string("test_server");

	ifstream in(strname.c_str(), ios::binary);
	MD5 md5(in);
	cout <<"md5:\t"<<md5.toString()<<endl;
	/*
	char m5[17];
	memcpy(m5, md5.digest(), 16);
	m5[17] = 0;
	cout <<"md5 :"<<m5<<endl;
	*/
	in.close();

	string result;

	_DFIELD_* pData = new _DFIELD_;
	pData->m_OperateType = DF_GET_FILE_MD5;
	pData->m_NO          = 0;
	sprintf(pData->m_Filename, "/opt/wangyt/UKS_DF_Proj/src/test_server");
	pData->m_DataLength = strlen(pData->m_Filename);
	result.append((const char*)&pData->m_OperateType, 4);
	result.append((const char*)&pData->m_NO, 4);
	result.append((const char*)&pData->m_DataLength, 4);
	result.append(pData->m_Filename, pData->m_DataLength);
	//result.append(md5.toString().c_str(), 32);

	Protocol head;	
	head.body_len = result.size();	

	string msg;	
	Protocol::HeadToBuffer(head, msg);	
	msg += result;

	//send package
	string err;
	int ret = tcp->send(msg, err);
	if(ret < 0){             
		cerr << " send failed : " << err << endl;     
		//assert(false);        
        //TODO send message
	}   


	//receive data
	char recvBuf[1024];
	memset(recvBuf, 0, sizeof(recvBuf));

	size_t len = 64;
	ret = tcp->recv(recvBuf, len, err);
	if(ret < 0){             
		cerr << " recv failed : " << err << endl;            
		//assert(false);        
	} 

	//parsing recv data
	string o;
	string strBuf = string(recvBuf, len);
	if(Protocol::ParseProtocol(strBuf, o) != PACKET_FULL)
	{
		printf("Package error\n");
		return -1;
	}

	if(o.length() < 12)
	{
		printf("Package body length error[%d]\n", (int)o.length());
		return -2;
	}
	
	const char *psBuf = o.c_str();
	int32_t Opt = *(int32_t *)psBuf;
	psBuf += 4;
	if(Opt != pData->m_OperateType)
	{
		printf("Opt error,Send [%d]--Recv [%d]\n", Opt, pData->m_OperateType);
		return -3;
	}

	int32_t NO = *(int32_t *)psBuf;
	psBuf += 4;
	if(NO != pData->m_NO)
	{
		printf("NO error,Send [%d]--Recv [%d]\n", NO, pData->m_NO);
		return -4;
	}

	pData->m_Result = *(uint32_t *)psBuf;
	psBuf += 4;

	if(pData->m_Result < 0)
		printf("md5 error!,ret:[%d]\n", pData->m_Result);
	printf("result:[%d]\n", pData->m_Result);
	
	char tmpmd5[33];
	memcpy(tmpmd5, psBuf, 32);
	tmpmd5[32] = 0;
	printf("get:%s\n", tmpmd5);
	
	delete pData;
	return 0;
}
