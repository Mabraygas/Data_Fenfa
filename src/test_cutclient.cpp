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
	line_vector.push_back("蜗居");
	line_vector.push_back("穆桂英挂帅第贰季");
	line_vector.push_back("电影甄嬛传八错，射雕英雄传也八错。");
	line_vector.push_back("Python is a great scripting language. Don't reinvent the wheel...your templates can handle it !");
	line_vector.push_back("理财教室-2008-04-刘建位-学习巴菲特系列之长期持有a");
	line_vector.push_back("蒙德鑫中国我哈哈Hello,我meme你他们.");
	line_vector.push_back("在纽约，一群Geek在街边发现了一台Macintosh SE，却未曾想到在这台年代久远的机器里，竟遗留下出乎意料的“彩蛋” 。");
	line_vector.push_back("Copies the first num characters of source to destination. If the end of the source C string (which is signaled by a null-character) is found before num characters have been copied, destination is padded with zeros until a total of num characters have been written to it.");
	line_vector.push_back("前段时间，我们提到了Mikengreg 公司两位开发者的故事，他们在开发了一个备受欢迎的应用之后，未能获得可观收入，反而弄得贫困潦倒。对于开发者来说，这可以说是一个值得思考的故事。");
	line_vector.push_back("周鸿祎四战搜索 百度内部高度紧张");
	line_vector.push_back("AMD Radeon HD 7000 系列显卡已经连续三轮大降价，而细心的朋友应该发现了，处理器方面已经很久很久没什么动静了。别急，该来的马上就来，而且规模超乎想象。");
	line_vector.push_back("挪威法院宣判涉及 77 条人命的枪手布雷维克恐怖罪行罪成入狱 21 年刑期可以延长. 在布雷维克进行自我陈述时， 他他的世界观来自维基百科.");
	line_vector.push_back("音乐问答游戏《你唱我猜》(SongPop)音乐问答游戏《你唱我猜》(SongPop)今年 5 月上线，它可以理解为音乐版的《你画我猜》，在 7 月到 9 月的两个月时间内日活跃用户翻了一番，达到 2500 万，社交游戏当前的火爆程度由此可见一斑。");

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

