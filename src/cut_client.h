#ifndef _CUT_CLIENT_H_
#define _CUT_CLIENT_H_

#include <eagle_thread.h>
#include <eagle_clientsocket.h>


using namespace std;
using namespace eagle;


/**
 * 切词客户端
 */
class CutClient {
public:

	/**
	 * @brief 构造函数
	 *
	 * @param [ip]      : 服务端ip
	 * @param [port]    : 服务端口
	 * @param [timeout] : 超时时间(ms)
	 */
	CutClient(const string &ip, int port, int timeout = 3000) :
	cs_(ip, port, timeout)
	{}

public:

	/**
	 * @brief 组包
	 */
	int MakeReqPkg(string & str, char * pkg);
	
	/**
	 *@brief 获取切词结果
	 *@param [str]     : 待切词的字符串
	 *@param [result]  : 切词结果
	 *@param [err]     : 错误信息
	 *@return -1: fail, 0: success
	 */
	int getCutRes(string &str, string &result, string &err);

	/**
	 *@brief 获取切词结果
	 *@param [str]     : 待切词的字符串
	 *@param [common]  : 普通切词结果
	 *@param [combine] : 复合切词结果
	 *@param [err]     : 错误信息
	 *@return -1: fail, 0: success
	 */
	int getCutRes(string &str, string &common, string &combine, string &err);

protected:

	/**
	 * @brief 客户端socket
	 */
	TCPClient cs_;
};

#endif
