HOME=/projects

INC=. ./include /projects/UKS_Lib_proj/src

LIBS=./lib/libeagle.a ./lib/libxml.a -L./lib/ -lmd5 -luks

BIN=DFSystem BaseUpdate DFS_DataClient DFS_DataServer DFS_IBServer
TOOL_BIN=xtool_md5builder xtool_dfscp xtool_getlocalip
TEST_BIN=test_server test_md5 test_monitor test_cutclient

#UKS=uks_str.o uks_net.o uks_mem.o uks_sms.o uks_time.o 
OBJS=protocol.o MemManger.o DayLog.o CountNumber.o
	 

#CFLAG=-ggdb -DNLOG
CFLAG=-ggdb -Wall -DNLOG


all: $(UKSOBJ) $(OUT_LIB) $(BIN) $(TOOL_BIN) $(TEST_BIN) 
#all: $(UKSOBJ) $(OUT_LIB) $(BIN)

DFSystem:QSEPSDF.o QSEPSDF_MainControl.o QSEPSDF_CS.o QSEPSDF_SS.o QSEPSDF_SSWork.o ThreadMd5.o SmsNotify.o work_client.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

DFS_DataClient:data_client.o QSEPSDF_CS.o QSEPSDF_SSWork.o QSEPSDF_SS.o QSEPSDF_MainControl.o ThreadMd5.o SmsNotify.o work_client.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

DFS_DataServer:data_server.o QSEPSDF_SSWork.o QSEPSDF_SS.o ThreadMd5.o SmsNotify.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

DFS_IBServer:IsBrokerServer.o IsBrokerWork.o ThreadMd5.o SmsNotify.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

verify_monitor:verify_monitor.o data_verify.o work_client.o SmsNotify.o QSEPSDF_MainControl.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

BaseUpdate:thread_update.o work_client.o SmsNotify.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

test_cutclient:test_cutclient.o cut_client.o
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

test_server:test_server.o QSEPSDF_CS.o QSEPSDF_MainControl.o work_client.o SmsNotify.o $(OBJS) 
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

test_md5:test_md5.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

test_monitor:test_monitor.o work_client.o symbol_monitor.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

xtool_dfscp:xtool_dfscp.o work_client.o $(OBJS)
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

xtool_md5builder:xtool_md5builder.o
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

xtool_getlocalip:xtool_getlocalip.o
	g++ $(CFLAG) -o $@  $^  $(addprefix -I, $(INC)) -lpthread $(LIBS)

.cpp.o:
	g++ $(CFLAG) -c $^  $(addprefix -I, $(INC))  

clean:
	rm -f *.o $(BIN) $(TOOL_BIN) $(TEST_BIN) $(OUT_LIB)

