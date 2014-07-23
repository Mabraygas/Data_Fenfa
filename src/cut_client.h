#ifndef _CUT_CLIENT_H_
#define _CUT_CLIENT_H_

#include <eagle_thread.h>
#include <eagle_clientsocket.h>


using namespace std;
using namespace eagle;


/**
 * �дʿͻ���
 */
class CutClient {
public:

	/**
	 * @brief ���캯��
	 *
	 * @param [ip]      : �����ip
	 * @param [port]    : ����˿�
	 * @param [timeout] : ��ʱʱ��(ms)
	 */
	CutClient(const string &ip, int port, int timeout = 3000) :
	cs_(ip, port, timeout)
	{}

public:

	/**
	 * @brief ���
	 */
	int MakeReqPkg(string & str, char * pkg);
	
	/**
	 *@brief ��ȡ�дʽ��
	 *@param [str]     : ���дʵ��ַ���
	 *@param [result]  : �дʽ��
	 *@param [err]     : ������Ϣ
	 *@return -1: fail, 0: success
	 */
	int getCutRes(string &str, string &result, string &err);

	/**
	 *@brief ��ȡ�дʽ��
	 *@param [str]     : ���дʵ��ַ���
	 *@param [common]  : ��ͨ�дʽ��
	 *@param [combine] : �����дʽ��
	 *@param [err]     : ������Ϣ
	 *@return -1: fail, 0: success
	 */
	int getCutRes(string &str, string &common, string &combine, string &err);

protected:

	/**
	 * @brief �ͻ���socket
	 */
	TCPClient cs_;
};

#endif
