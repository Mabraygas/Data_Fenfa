#ifndef _PROTOCOL_H
#define _PROTOCOL_H
/**
 * @file protocol.h
 * @brief Э��
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
 * EagleЭ��
 *
 * ʹ�ü��:
 * 1) KEY
 * ����ʱ�����ݰ���key�ֶ���Protocol::KEY�����,����ΪЭ�����(PACKET_ERR).
 * Ĭ��ֵ��0xfb70939,���޸�:
 * Protocol::KEY = xxx;
 *
 * 2) WHO
 * �������ݰ�ʱ�õ�.
 * Ĭ��Ϊ��,���޸�:
 * Protocol::WHO = "xxx";
 *
 * 3) VERSION
 * �������ݰ�ʱ�õ�.
 * Ĭ��Ϊ0x01,���޸�:
 * Protocol::VERSION = xxx;
 *
 * 4) �������������ݽ���ΪProtocol����(�������ݰ�������ʱ)
 * static void BufferToHead(const std::string &buffer, Protocol &head);
 *
 * 5) ��Protocol��������������(�������ݰ�ʱ)
 * static void HeadToBuffer(const Protocol &head, std::string &buffer);
 *
 * 6) EagleЭ�������(����server)
 * static int ParseProtocol(std::string &buffer, std::string &o);
 * ʹ��: adapter->setProtocol(Protocol::ParseProtocol);
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
	 * @brief ��ͷ����
	 */
	const static size_t LEN = sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint32_t);

public:

	/**
	 * @brief �������������ݽ���ΪProtocol����.
	 * ע��: 
	 * �ô˺���BufferToHead(string,Protocol)ʱҪ����ע�⴫��char��������!
	 * char������������Ƶ�����,����ʱ�ᱻת��Ϊstring����,���'\0'��������ݻᱻ�ض�!!!
	 *
	 * @param [buffer] : Ҫ�����Ļ�����,�ɵ����ߴ���.����Ҫ��֤����(>= Protocol::LEN)
	 * @param [head]   : �������֮�������,�ɵ����ߴ���.
	 */
	static void BufferToHead(const char *buffer, size_t buflen, Protocol &head);

	/**
	 * @brief ��Protocol��������������
	 *
	 * @param [head]   : Protocol����
	 * @param [buffer] : �������ݵĻ�����,�ɵ����ߴ���.�����ɱ���������.
	 */
	static void HeadToBuffer(const Protocol &head, std::string &buffer);

public:

	/**
	 * @brief Ĭ�Ϲ��캯��
	 */
	Protocol();

	/**
	 * @brief �������캯��
	 *
	 * @param [head] : ��һProtocol����
	 */
	Protocol(const Protocol &head);

	/**
	 * @brief ��ֵ���캯��
	 *
	 * @param [head] : ��һProtocol����
	 *
	 * @return : ��ǰ���������
	 */
	Protocol &operator=(const Protocol &head);

public:

	/**
	 * @brief EagleЭ�������(����server)
	 *
	 * @param [buffer] : Conn�����client���յ�������,���ܲ���һ�����������ݰ�,�������ж�.
	 * @param [o]      : �������.
	 *
	 * @return : �����Ľ��,������:
	 * PACKET_LESS : �յ��İ���ȫ;
	 * PACKET_FULL : �Ѿ��յ������İ�(��bufferȥ��������������);
	 * PACKET_ERR  : Э�����(��bufferȥ��������������).
	 */
	static int ParseProtocol(std::string &buffer, std::string &o);

private:

	/**
	 * @brief ����
	 *
	 * @param [head] : ��һProtocol����
	 */
	void copy(const Protocol &head);
};

#endif

