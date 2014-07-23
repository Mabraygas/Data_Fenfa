#ifndef EAGLE_EPOLLSERVER_H
#define EAGLE_EPOLLSERVER_H
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <string>
#include <sstream>

#ifndef NLOG
#include <log4cplus/logger.h>
#include <log4cplus/streams.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/consoleappender.h>
#endif

#include "eagle_lock.h"
#include "eagle_signal.h"
#include "eagle_socket.h"
#include "eagle_thread.h"
#include "eagle_atomic.h"
#include "eagle_epoller.h"
#include "eagle_autoptr.h"
#include "eagle_endpoint.h"
#include "eagle_thread_queue.h"

namespace eagle
{
class Work;
class Conn;
class BindAdapter;
class WorkGroup;
class EpollServer;

typedef AutoPtr<Work> 	     WorkPtr;
typedef AutoPtr<WorkGroup>   WorkGroupPtr;
typedef AutoPtr<BindAdapter> BindAdapterPtr;
typedef AutoPtr<EpollServer> EpollServerPtr;
struct RecvData: public AutoPtrBase
{
	uint32_t        uid;            
	std::string     buffer;         
	std::string     ip;             
	uint16_t        port;           
	time_t          timestamp;  	
	bool            is_overload; 	
	bool            is_closed; 		
	BindAdapterPtr  adapter;        
};
typedef AutoPtr<RecvData> RecvDataPtr;
typedef ThreadQueue<RecvDataPtr, std::deque<RecvDataPtr> > recv_queue;
typedef recv_queue::queue_type recv_queue_type;

struct SendData: public AutoPtrBase
{
	char            cmd;            
	uint32_t        uid;            
	std::string     buffer;         
	std::string     ip;             
	uint16_t        port;           
};
typedef AutoPtr<SendData> SendDataPtr;
typedef ThreadQueue<SendDataPtr, std::deque<SendDataPtr> > send_queue;

enum 
{
	PACKET_LESS = 0,    
	PACKET_FULL = 1,    
	PACKET_ERR  = -1,   
};

typedef int (*ProtocolFunctor)(std::string &buffer, std::string &o);

#ifndef NLOG

#define MDX_TRACE(p) \
	LOG4CPLUS_TRACE(pEpollServer_->get_logger(),p)

#define MDX_DEBUG(p) \
	LOG4CPLUS_DEBUG(pEpollServer_->get_logger(),p)

#define MDX_INFO(p) \
	LOG4CPLUS_INFO(pEpollServer_->get_logger(),p)
#define MDX_NOTICE(p) \
	LOG4CPLUS_INFO(pEpollServer_->get_logger(),p)

#define MDX_WARN(p) \
	LOG4CPLUS_WARN(pEpollServer_->get_logger(),p)
#define MDX_WARNING(p) \
	LOG4CPLUS_WARN(pEpollServer_->get_logger(),p)

#define MDX_ERROR(p) \
	LOG4CPLUS_ERROR(pEpollServer_->get_logger(),p)

#define MDX_FATAL(p) \
	LOG4CPLUS_FATAL(pEpollServer_->get_logger(),p)

#else

#define MDX_TRACE(p) 	;
#define MDX_DEBUG(p) 	;
#define MDX_INFO(p)  	;
#define MDX_NOTICE(p) 	;
#define MDX_WARN(p) 	;
#define MDX_WARNING(p)  ;
#define MDX_ERROR(p) 	;
#define MDX_FATAL(p) 	;

#endif
class BindAdapter : public ThreadLock, public AutoPtrBase
{
public:

	BindAdapter(EpollServer   *pEpollServer = NULL);
	BindAdapter(EpollServerPtr pEpollServer = NULL);

	~BindAdapter();

	void setEpollServer(EpollServer *pEpollServer) {pEpollServer_ = pEpollServer;}

	EpollServer* getEpollServer() const {return pEpollServer_;}

	void setName(const std::string &name) {name_ = name;}

	std::string getName() const {return name_;};

	void setEndpoint(const std::string &str);

	void setEndpoint(const Endpoint &ep);

	Endpoint &getEndpoint() {return ep_;}

	void bind();

	Socket &getSocket() {return socket_;}

	void setMaxConns(int max_conns) {max_conns_ = max_conns;}

	int getMaxConns() const {return max_conns_;}

	bool IsLimitMaxConn() const {return curr_conns_ + 1 > max_conns_;}

	size_t getNowConn() const {return curr_conns_.get();}

	void IncreaseNowConn() {curr_conns_.inc();}

	void DecreaseNowConn() {curr_conns_.dec();}

	void setProtocolName(const std::string& name);

	std::string& getProtocolName() {return protocol_name_;}

	void setProtocol(const ProtocolFunctor &pf) {pf_ = pf;}

	ProtocolFunctor &getProtocol() {return pf_;}

	static int EchoProtocol(std::string &buffer, std::string &o);

	void InsertRecvQueue(const recv_queue::queue_type &recv_data_queue, bool push_back = true);

	bool WaitForRecvQueue(RecvDataPtr &recv, int wait_time);

	size_t getRecvBufferSize() const {return rbuffer_.size();}

	void setQueueCapacity(int n) {queue_capacity_ = n;}

	int getQueueCapacity() const {return queue_capacity_;}

	bool IsOverload() {return queue_capacity_ > 0 && (int)rbuffer_.size() > queue_capacity_;}

	void setQueueTimeout(int t) {queue_timeout_ = (t > MIN_QUEUE_TIMEOUT ? t : MIN_QUEUE_TIMEOUT);}

	int getQueueTimeout() const {return queue_timeout_;}

	void setWorkGroupName(const std::string& work_group_name) {work_group_name_ = work_group_name;}

	std::string &getWorkGroupName() {return work_group_name_;}

	void setWorkNum(int n);

	int getWorkNum() const {return work_num_;}

public:

	enum
	{
		DEFAULT_QUEUE_CAP		= 10*1024,    
		MIN_QUEUE_TIMEOUT		= 3*1000,     
		DEFAULT_MAX_CONN		= 1024,       
		DEFAULT_QUEUE_TIMEOUT	= 60*1000,    
	};

protected:

	friend class EpollServer;

	EpollServer *pEpollServer_;

	std::string name_;

	Endpoint ep_;

	Socket socket_;

	int max_conns_;

	Atomic curr_conns_;

	std::string protocol_name_;

	ProtocolFunctor pf_;

	recv_queue rbuffer_;

	int queue_capacity_;

	int queue_timeout_;

	std::string work_group_name_;

	WorkGroupPtr work_group_;

	size_t work_num_;
};
class Conn
{
public:

	Conn(BindAdapter *pBindAdapter, int fd, int timeout, const std::string& ip, uint16_t port);

	 ~Conn();

	 BindAdapter* getBindAdapter() {return pBindAdapter_;}

	void init(uint32_t uid) { uid_ = uid;}

	 uint32_t getId() const  { return uid_; }

	int getfd() const { return sock_.getfd(); }

	std::string getIp() const { return ip_; }

	uint16_t getPort() const { return port_; }

	bool setClose() { bClose_ = true; return send_buffer_.empty(); }

	int getTimeout() const {return timeout_;}

protected:

	int recv(recv_queue::queue_type &o);

	int send(const std::string& buffer, const std::string &ip, uint16_t port);

	int send();
	
	void close();

private:

	int ParseProtocol(recv_queue::queue_type &o);

	void InsertRecvQueue(recv_queue::queue_type &recv_data_queue);

public:

	time_t last_refresh_time;

protected:

	friend class EpollServer;

	BindAdapter *pBindAdapter_;

	EpollServer  *pEpollServer_;
	
	Socket sock_;

	uint32_t uid_;

	std::string ip_;

	uint16_t port_;

	std::string recv_buffer_;

	std::string send_buffer_;

	size_t send_pos_;

	bool bClose_;
	
	int timeout_;

	int max_tem_queue_size_;
};
class ConnList : public ThreadLock
{
public:

	ConnList(EpollServer *pEpollServer): pEpollServer_(pEpollServer),connection_array_(NULL) {}

	~ConnList()
	{if(connection_array_) {delete []connection_array_;}}

	void init(uint32_t size);

	uint32_t GetUniqId();

	void AddConn(Conn *conn);

	void DelConn(uint32_t uid);

	void Refresh(uint32_t uid);

	void CheckTimeOut();

	Conn *get(uint32_t uid);

	size_t size();

protected:

	void DelConnNoLock(uint32_t index);

protected:

	typedef std::pair<Conn*, std::multimap<time_t, uint32_t>::iterator> ConnArrayEleType;

protected:

	EpollServer *pEpollServer_;

	uint32_t capacity_;

	ConnArrayEleType *connection_array_;

	std::list<uint32_t> free_list_;

	size_t free_size_;

	std::multimap<time_t, uint32_t> timeout_map_;

	time_t last_check_time_;

	uint32_t magic_;
};
class Work : public Thread, public ThreadLock, public AutoPtrBase
{
public:

	Work();

	virtual ~Work() {}

	void setEpollServer(EpollServer *pEpollServer);

	EpollServer* getEpollServer() {return pEpollServer_;}

	void setWorkGroup(WorkGroupPtr& work_group);

	WorkGroupPtr& getWorkGroup() {return work_group_;}

	void setWaitTime(int wait_time);

public:

	void SendResponse(uint32_t uid, const std::string &buffer, const std::string &ip, uint16_t port);

	
	void SendResponseEagleProtocol(uint32_t uid,const std::string &body,const std::string &ip,uint16_t port);

	void close(uint32_t uid);

protected:

	virtual void run();

	virtual void initialize() {};

	virtual void WorkImp();

	virtual void startWork() {}

	virtual void stopWork() {}

	virtual void HeartBeat() {}

	virtual void WorkFilter() {}

	virtual bool AdapterIsEmpty();

	virtual bool FilterIsEmpty() {return true;}

protected:

	virtual void WorkOverload(const RecvData &recv);

	virtual void WorkClose(const RecvData &recv) {}

	virtual void WorkTimeout(const RecvData &recv);

protected:

	virtual void work(const RecvData &recv) = 0;

protected:

	EpollServer  *pEpollServer_;

	WorkGroupPtr work_group_;

	int wait_time_;
};
struct WorkGroup : public AutoPtrBase
{
	std::string 			name;		
	ThreadLock				monitor;	
	std::vector<WorkPtr>	works;		
	BindAdapterPtr			adapter;	
};
class EpollServer : public ThreadLock, public AutoPtrBase, public SignalHandler
{
public:

	EpollServer();

	virtual ~EpollServer();

	template<class T>
	bool CreateWorkGroup(const std::string &group_name, int32_t work_num, BindAdapterPtr adapter);

	bool CreateWorkGroup(const std::string &group_name, std::vector<WorkPtr> &works, BindAdapterPtr adapter);

	void WaitForShutdown();

	void Terminate();

	bool IsTerminate() const {return terminated_;}

	void setClose()
	{b_close_ = true;}

	void send(uint32_t uid, const std::string &buffer, const std::string &ip, uint16_t port);
	void send(uint32_t uid, const std::string &buffer) {send(uid,buffer,"",0);}
	void sendEagleProtocol(uint32_t uid, const std::string &buffer);

	void close(uint32_t uid);

	Conn *getConnectionPtr(uint32_t uid)
	{return list_.get(uid);}

public:
#ifndef NLOG
	void set_logger(log4cplus::Logger &logger){logger_ = logger;}

	log4cplus::Logger &get_logger(){return logger_;}

	void SetConsoleLogger(const std::string &level = "TRACE");

	void SetRollingFileLog(const std::string &filename, 
			 			   long  max_file_size      = 1024*1024*1024, 
						   int   max_backup_index   = 3, 
						   const std::string &level = "TRACE");

	void SetDailyRollingFileLog(const std::string &filename, 
								log4cplus::DailyRollingFileSchedule schedule = log4cplus::DAILY,
						   		int   max_backup_index   = 3, 
						   		const std::string &level = "TRACE");
#endif
protected:

	void StartWork();

	void StopWork();

	void CreateEpoll();

protected:

	bool accept(int fd);

	void ProcessNet(const epoll_event &ev);

	void ProcessPipe();

protected:

	void AddTcpConn(Conn *conn);

	void DelConn(Conn *conn, bool timeout_erased = false);

protected:

	void 
	CreateWorkGroupImp(std::vector<WorkPtr> &works, const std::string &group_name, BindAdapterPtr adapter);

protected:
#ifndef NLOG
	void CreateLogger(log4cplus::SharedAppenderPtr &appender, log4cplus::Logger &logger, const std::string &level);
#endif

	friend class Work;
	friend class Conn;
	friend class BindAdapter;
	friend class ConnList;

protected:

	virtual void HandleSignal(int signo);

protected:

	enum
	{
		ET_LISTEN = 1, 
		ET_CLOSE  = 2, 
		ET_NOTIFY = 3, 
		ET_NET    = 0, 
	};

protected:

	std::map<std::string, WorkGroupPtr> work_groups_;

	std::map<int, BindAdapterPtr> listeners_;

	bool work_started_;

	Epoller epoller_;

	Socket notify_;

	Socket shutdown_;

	ConnList list_;

	bool terminated_;

	bool b_close_;

	send_queue pipe_buffer_;

#ifndef NLOG
	log4cplus::Logger logger_;
#endif

	EpollServer  *pEpollServer_;
};

template<class T>
bool EpollServer::CreateWorkGroup(const std::string &group_name, int32_t work_num, BindAdapterPtr adapter)
{
	std::map<std::string, WorkGroupPtr>::iterator it = work_groups_.find(group_name); 
	if(it != work_groups_.end()){
		MDX_FATAL("exist the same group_name : " << group_name);
		return false;
	}

	std::vector<WorkPtr> works;
	for(int32_t i = 0; i < work_num; ++i){
		WorkPtr work = new T();
		works.push_back(work);
	}
	MDX_TRACE("work thread num : " << work_num);

	CreateWorkGroupImp(works,group_name,adapter);

	return true;
}

}

#endif
