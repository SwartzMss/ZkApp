#include "zookeeper.hpp"
#include "LogDef.h"
#include "zookeeper.hpp"

using namespace zookeeper;
int main()
{
	const char* host = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    int timeout = 30000;
    
	ZooKeeper zk;
	zk.StartService(host, NULL, timeout, ZOO_LOG_LEVEL_INFO);
	sleep(3);
	LOG_INFO("status = %d ", zk.is_connected());
	return 0;
}