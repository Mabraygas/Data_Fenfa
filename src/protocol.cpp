#include "protocol.h"
#include "Eps_WIN32_LINUX.h"

#define HEAD_APP_ID     0xDFDF0101

const size_t Protocol::LEN;

void Protocol::copy(const Protocol &head)
{
	id       = head.id;
	app_id   = head.app_id;
	func     = head.func;
	body_len = head.body_len;
}

Protocol::Protocol() : id(1), app_id(HEAD_APP_ID), func(FUN_DF)
{ }

Protocol::Protocol(const Protocol &head)
{
	if(this == &head)
		return;
	copy(head);
}

Protocol &Protocol::operator=(const Protocol &head)
{
	if(this == &head)
		return *this;
	copy(head);
	return *this;
}

void Protocol::BufferToHead(const char *mbuffer, size_t buflen, Protocol &head)
{
	//string buffer(mbuffer, buflen);
	/*if(buffer.length() < LEN){
		return;
	}*/

	const char *_buffer = mbuffer;//buffer.data();
	size_t len;
	size_t pos = 0;

	//id
	len = sizeof(uint64_t);
	memcpy(&(head.id), _buffer+pos, len);
	pos += len;

	len = sizeof(uint32_t);
	memcpy(&(head.app_id), _buffer+pos, len);
	pos += len;

	len = sizeof(uint32_t);
	memcpy(&(head.func), _buffer+pos, len);
	pos += len;

	//body_len
	len = sizeof(uint32_t);
	memcpy(&(head.body_len), _buffer+pos, len);
	pos += len;
}

void Protocol::HeadToBuffer(const Protocol &head, string &buffer)
{
	 buffer = "";

	 //id
	 buffer.append((const char *)&(head.id),sizeof(head.id));

	 //app_id
	 buffer.append((const char *)&(head.app_id),sizeof(head.app_id));

	 //func
	 buffer.append((const char *)&(head.func),sizeof(head.func));

	 //body_len
	 buffer.append((const char *)&(head.body_len),sizeof(head.body_len));
}

int Protocol::ParseProtocol(string &buffer, string &o)
{
	//�ж�buffer�����Ƿ���ڰ�ͷ����
	if(buffer.length() <= LEN){//��ͷ��ȫ
		return PACKET_LESS;
	}

	//�������������ݽ�����Protocol����
	Protocol head;
	BufferToHead(buffer.c_str(),buffer.size(),head);

	//�ж�buffer�����Ƿ����������(��ͷ+����)�ĳ���
	size_t pack_len = LEN + head.body_len;
	if(buffer.length() < pack_len){//���岻ȫ
		return PACKET_LESS;
	}

	//��ȡ���������
	o = buffer.substr(LEN,head.body_len);

	//��buffer��ȥ�������ݰ�
	buffer = buffer.substr(pack_len, buffer.length()-pack_len);

	//���������İ� 
	return PACKET_FULL;
}
