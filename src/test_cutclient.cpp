#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <eagle_clientsocket.h>

#include "cut_client.h"

using namespace std;
using namespace eagle;


//////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

	printf("%s ip port timeout\n", argv[0]);

	//string ip = "127.0.0.1";
	string ip = "10.103.8.66";
	int port = 19222;
	int thread_num = 1;
	int timeout = 3000;
	if (argc >= 4) {
		ip = string(argv[1]);
		port = atoi(argv[2]);
		timeout = atoi(argv[3]);
	}
	cout << "ip         : " << ip << endl;
	cout << "port       : " << port << endl;
	cout << "timeout    : " << timeout << endl;

	vector<string> line_vector;
	line_vector.push_back("�Ͼ�");
	line_vector.push_back("�¹�Ӣ��˧�ڷ���");
	line_vector.push_back("��Ӱ��ִ��˴����Ӣ�۴�Ҳ�˴�");
	line_vector.push_back("Python is a great scripting language. Don't reinvent the wheel...your templates can handle it !");
	line_vector.push_back("��ƽ���-2008-04-����λ-ѧϰ�ͷ���ϵ��֮���ڳ���a");
	line_vector.push_back("�ɵ����й��ҹ���Hello,��meme������.");
	line_vector.push_back("��ŦԼ��һȺGeek�ڽֱ߷�����һ̨Macintosh SE��ȴδ���뵽����̨�����Զ�Ļ�����������³������ϵġ��ʵ��� ��");
	line_vector.push_back("Copies the first num characters of source to destination. If the end of the source C string (which is signaled by a null-character) is found before num characters have been copied, destination is padded with zeros until a total of num characters have been written to it.");
	line_vector.push_back("ǰ��ʱ�䣬�����ᵽ��Mikengreg ��˾��λ�����ߵĹ��£������ڿ�����һ�����ܻ�ӭ��Ӧ��֮��δ�ܻ�ÿɹ����룬����Ū��ƶ���ʵ������ڿ�������˵�������˵��һ��ֵ��˼���Ĺ��¡�");
	line_vector.push_back("�ܺ�t��ս���� �ٶ��ڲ��߶Ƚ���");
	line_vector.push_back("AMD Radeon HD 7000 ϵ���Կ��Ѿ��������ִ󽵼ۣ���ϸ�ĵ�����Ӧ�÷����ˣ������������Ѿ��ܾúܾ�ûʲô�����ˡ��𼱣����������Ͼ��������ҹ�ģ��������");
	line_vector.push_back("Ų����Ժ�����漰 77 ��������ǹ�ֲ���ά�˿ֲ������������ 21 �����ڿ����ӳ�. �ڲ���ά�˽������ҳ���ʱ�� ���������������ά���ٿ�.");
	line_vector.push_back("�����ʴ���Ϸ���㳪�Ҳ¡�(SongPop)�����ʴ���Ϸ���㳪�Ҳ¡�(SongPop)���� 5 �����ߣ����������Ϊ���ְ�ġ��㻭�Ҳ¡����� 7 �µ� 9 �µ�������ʱ�����ջ�Ծ�û�����һ�����ﵽ 2500 ���罻��Ϸ��ǰ�Ļ𱬳̶��ɴ˿ɼ�һ�ߡ�");

	CutClient *pcc = new CutClient(ip, port, timeout);
	string result, err;
	string common, combine;

	for (vector<string>::iterator it = line_vector.begin();
			it != line_vector.end(); ++it) {
		pcc->getCutRes(*it, common, combine, err);
		printf("str:'%s',\n common:'%s'\ncombine:'%s'\n", it->c_str(), common.c_str(), combine.c_str());
	}

	delete pcc;
	return 0;
}

