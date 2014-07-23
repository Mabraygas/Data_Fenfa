#include <stdio.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>


//��ȡ��ַ
//����IP��ַ�ַ���
//���أ�0=�ɹ���-1=ʧ��
int getlocalip(char* outip)
{
	int i=0;
	int sockfd;
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	char* ip;
	//��ʼ��ifconf
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}

	ioctl(sockfd, SIOCGIFCONF, &ifconf); //��ȡ���нӿ���Ϣ
	close(sockfd);

	//������һ��һ���Ļ�ȡIP��ַ
	ifreq = (struct ifreq*)buf;
	for( i = (ifconf.ifc_len/sizeof(struct ifreq)); i > 0; i++)
	{
		ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);

		if(strcmp(ip,"127.0.0.1")==0) //�ų�127.0.0.1��������һ��
		{
			ifreq++;
			continue;
		}
		strcpy(outip,ip);
		return 0;
	}

	return -1;
}

int main(void)
{
	char ip[20];

	if ( getlocalip( ip ) == 0 )
	{
		printf("����IP��ַ�ǣ� %s\n", ip );
	}
	else
	{
		printf("�޷���ȡ����IP��ַ\n");
	}
	return 0;
}
