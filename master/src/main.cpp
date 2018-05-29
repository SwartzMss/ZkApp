#include "zookeeper.h"
#include "LogDef.h"

#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;


#include "unistd.h"

static  int ConnectStat = -1;
static  int  server_id ;
zhandle_t* zkhandle = NULL;

void QueryServer_watcher_g(zhandle_t* zh, int type, int state,const char* path, void* watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) 
	{
        if (state == ZOO_CONNECTED_STATE) 
		{
			ConnectStat = 1;
            LOG_INFO("Connected to zookeeper service successfully!");
        } 
		else if (state == ZOO_EXPIRED_SESSION_STATE) 
		{ 
			ConnectStat = 0;
            LOG_ERROR("Zookeeper session expired!");
        }
    }  
}

void QueryServer_string_completion(int rc, const char *name, const void *data)
{
   switch (rc) 
   {
		case ZCONNECTIONLOSS:
			LOG_ERROR("ZCONNECTION LOSS ");
			break;
		case ZOK:
			LOG_INFO("Created  node %s success", name);
			break;
		case ZNODEEXISTS:
			LOG_ERROR("Node %s already exists",(char*)data);
			break;
		default:
			LOG_ERROR("Something went wrong when Created  node %s" ,data);
			break;
	}
}

#if 0
void master_exists() 
{
	zoo_awexists(zkhandle,"/master",master_exists_watcher,NULL,master_exists_completion,NULL);
}

void master_create_completion (int rc, const char *value, const void *data)
{
	switch (rc)
	{
		case ZCONNECTIONLOSS:
			LOG_ERROR("ZCONNECTION LOSS ");
			break;
		case ZOK:
			take_leadership();
			break;
		case ZNODEEXISTS:
			master_exists();
			break;
		default:
			LOG_ERROR("Something went wrong when running for master");
			break;
	}
}

void run_for_master() 
{
	char server_id_string[9];
	snprintf(server_id_string, 9, "%x", server_id);
	zoo_acreate(zkhandle,"/master",(const char *) server_id_string,sizeof(int),&ZOO_OPEN_ACL_UNSAFE,ZOO_EPHEMERAL,master_create_completion,NULL);
}
#endif

int main()
{
	const char* host = "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183";
    int timeout = 30000;
    
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
	
    zkhandle = zookeeper_init(host,QueryServer_watcher_g, timeout, 0, 0, 0);
    if (zkhandle == NULL) 
	{
        LOG_ERROR("Error when connecting to zookeeper servers...\n");
        exit(EXIT_FAILURE);
    }
	LOG_INFO("zookeeper_init success ");
	
	server_id = rand();
	while(1)
	{
		if(ConnectStat != -1)
		{
			break;
		}
		sleep(1);
	}
	
	if(ConnectStat != 1)
	{
		exit(EXIT_FAILURE);
	}
	
	int ret = zoo_acreate(zkhandle, "/Workers", "", 0,&ZOO_OPEN_ACL_UNSAFE, 0,QueryServer_string_completion, "/Workers");
    if (ret) 
	{
        LOG_ERROR("zoo_acreate error ,code = %d", ret);
        exit(EXIT_FAILURE);
    }
	
	ret = zoo_acreate(zkhandle, "/Jobers", "", 0,&ZOO_OPEN_ACL_UNSAFE, 0,QueryServer_string_completion, "/Workers");
    if (ret) 
	{
        LOG_ERROR("zoo_acreate error ,code = %d", ret);
        exit(EXIT_FAILURE);
    }
	
	sleep(5);
	zookeeper_close(zkhandle);
	return 0;
}