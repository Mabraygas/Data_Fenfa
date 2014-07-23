#ifndef _PROTOCOL_H
#define _PROTOCOL_H
/**
 * @file protocol.h
 * @brief 协议
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-09-25
 */
#include <string>
#include <string.h>
#include <stdint.h>

#include <sys/time.h>
#include <arpa/inet.h>

#include <eagle_epoll_server.h>

using namespace std;
using namespace eagle;

/**
 * Eagle协议
 *
 * 使用简介:
 * 1) KEY
 * 接收时若数据包的key字段与Protocol::KEY不相等,则视为协议错误(PACKET_ERR).
 * 默认值是0xfb70939,可修改:
 * Protocol::KEY = xxx;
 *
 * 2) WHO
 * 发送数据包时用到.
 * 默认为空,可修改:
 * Protocol::WHO = "xxx";
 *
 * 3) VERSION
 * 发送数据包时用到.
 * 默认为0x01,可修改:
 * Protocol::VERSION = xxx;
 *
 * 4) 将缓冲区的内容解析为Protocol对象(接收数据包并解析时)
 * static void BufferToHead(const std::string &buffer, Protocol &head);
 *
 * 5) 将Protocol对象打包到缓冲区(发送数据包时)
 * static void HeadToBuffer(const Protocol &head, std::string &buffer);
 *
 * 6) Eagle协议解析器(用于server)
 * static int ParseProtocol(std::string &buffer, std::string &o);
 * 使用: adapter->setProtocol(Protocol::ParseProtocol);
 */
struct Protocol
{
public:

	uint64_t id;
	
	uint32_t app_id;

	uint32_t func;

	uint32_t body_len;

public:

	/**
	 * @brief 包头长度
	 */
	const static size_t LEN = sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);

public:

	/**
	 * @brief 将缓冲区的内容解析为Protocol对象.
	 * 注意: 
	 * 用此函数BufferToHead(string,Protocol)时要格外注意传入char数组的情况!
	 * char数组包含二进制的数据,传入时会被转化为string类型,因此'\0'后面的数据会被截断!!!
	 *
	 * @param [buffer] : 要解析的缓冲区,由调用者创建.长度要保证够长(>= Protocol::LEN)
	 * @param [head]   : 保存解析之后的内容,由调用者创建.
	 */
	static void BufferToHead(const char *buffer, size_t buflen, Protocol &head);

	/**
	 * @brief 将Protocol对象打包到缓冲区
	 *
	 * @param [head]   : Protocol对象
	 * @param [buffer] : 保存数据的缓冲区,由调用者创建.长度由本函数控制.
	 */
	static void HeadToBuffer(const Protocol &head, std::string &buffer);

public:

	/**
	 * @brief 默认构造函数
	 */
	Protocol();

	/**
	 * @brief 拷贝构造函数
	 *
	 * @param [head] : 另一Protocol对象
	 */
	Protocol(const Protocol &head);

	/**
	 * @brief 赋值构造函数
	 *
	 * @param [head] : 另一Protocol对象
	 *
	 * @return : 当前对象的引用
	 */
	Protocol &operator=(const Protocol &head);

public:

	/**
	 * @brief Eagle协议解析器(用于server)
	 *
	 * @param [buffer] : Conn对象从client接收到的内容,可能不是一个完整的数据包,须自行判断.
	 * @param [o]      : 保存包体.
	 *
	 * @return : 解析的结果,有三种:
	 * PACKET_LESS : 收到的包不全;
	 * PACKET_FULL : 已经收到完整的包(从buffer去除整个包的内容);
	 * PACKET_ERR  : 协议错误(从buffer去除整个包的内容).
	 */
	static int ParseProtocol(std::string &buffer, std::string &o);

private:

	/**
	 * @brief 复制
	 *
	 * @param [head] : 另一Protocol对象
	 */
	void copy(const Protocol &head);
};

#endif

