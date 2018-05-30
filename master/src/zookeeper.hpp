#ifndef  ZOOK_KEEPER_HPP
#define  ZOOK_KEEPER_HPP

#include "ZKDefine.h"


/*默认只能单线程去执行 ,多线程非安全*/

namespace zookeeper 
{

string ConvertCodeToMsg(int ErrCode);

string ConvertStateToMsg(int state);

string ConvertEventToMsg(int event);


typedef Stat NodeStat;

class ZooWatcher 
{
public:
	virtual ~ZooWatcher() {}

	virtual void OnConnected() = 0;
	virtual void OnConnecting() = 0;
	virtual void OnSessionExpired() = 0;

	virtual void OnCreated(const char* path) = 0;
	virtual void OnDeleted(const char* path) = 0;
	virtual void OnChanged(const char* path) = 0;
	virtual void OnChildChanged(const char* path) = 0;
	virtual void OnNotWatching(const char* path) = 0;
};

class ZooKeeper 
{
public:
	ZooKeeper();

	~ZooKeeper();

	int StartService(const std::string& server_hosts, ZooWatcher* global_watcher = nullptr, int timeout_ms = 5 * 1000, ZooLogLevel level = ZOO_LOG_LEVEL_INFO);

	void StopService();

private:
	//禁止拷贝构造
	ZooKeeper(const ZooKeeper&) {};
	ZooKeeper& operator=(const ZooKeeper&) {};

public:
	bool is_connected();
	bool is_expired();

	bool Exists(const std::string& path,  NodeStat* stat,bool watch = false);

	std::string Create(const std::string& path,const std::string& value = std::string(),int flag = 0);

	std::string CreateIfNotExists(const std::string& path,const std::string& value = std::string(),int flag = 0);

	void Delete(const std::string& path);

	void DeleteIfExists(const std::string& path);

	void Set(const std::string&path, const std::string& value);

	std::string Get(const std::string& path, bool watch = false);

	std::vector<std::string> GetChildren(const std::string& parent_path, bool watch = false);

public:
	FILE * m_logFile;

	zhandle_t* m_zoo_handle_ ;

	ZooWatcher* m_global_watcher_ ;

	void WatchHandler(int type, int state, const char* path);

	static void GlobalWatchFunc(zhandle_t*, int type, int state,const char* path, void* ctx);
};


}







#endif