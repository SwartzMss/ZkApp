#include "zookeeper.hpp"

#include "LogDef.h"

namespace zookeeper 
{

	string ConvertCodeToMsg(int ErrCode)
	{
		switch(ErrCode)
		{
		case ZOK:
			return "Everything is OK";
		case ZSYSTEMERROR:
			return "System error";
		case ZRUNTIMEINCONSISTENCY:
			return "A runtime inconsistency was found";
		case ZDATAINCONSISTENCY:
			return "A data inconsistency was found";
		case ZCONNECTIONLOSS:
			return "Connection to the server has been lost";
		case ZMARSHALLINGERROR:
			return "Error while marshalling or unmarshalling data";
		case ZUNIMPLEMENTED:
			return "Operation is unimplemented";
		case ZOPERATIONTIMEOUT:
			return "Operation timeout";
		case ZBADARGUMENTS:
			return "Invalid arguments";
		case ZINVALIDSTATE:
			return "Invalid zhandle state";
		case ZAPIERROR:
			return "Api error";
		case ZNONODE:
			return "Node does not exist";
		case ZNOAUTH:
			return "Not authenticated";
		case ZBADVERSION:
			return "Version conflict";
		case ZNOCHILDRENFOREPHEMERALS:
			return "Ephemeral nodes may not have children";
		case ZNODEEXISTS:
			return "The node already exists";
		case ZNOTEMPTY:
			return "The node has children";
		case ZSESSIONEXPIRED:
			return "The session has been expired by the server";
		case ZINVALIDCALLBACK:
			return "Invalid callback specified";
		case ZINVALIDACL:
			return "Invalid ACL specified";
		case ZAUTHFAILED:
			return "Client authentication failed";
		case ZCLOSING:
			return "ZooKeeper is closing";
		case ZNOTHING:
			return "(not error) no server responses to process";
		case ZSESSIONMOVED:
			return "Session moved to another server, so operation is ignored";
		default:
			return "unknown error";
		}
	}

	string ConvertEventToMsg(int event)
	{
		if(ZOO_CREATED_EVENT == event)
			return "ZOO_CREATED_EVENT";
		else if(ZOO_DELETED_EVENT == event)
			return "ZOO_DELETED_EVENT";
		else if(ZOO_CHANGED_EVENT == event)
			return "ZOO_CHANGED_EVENT";
		else if(ZOO_SESSION_EVENT == event)
			return "ZOO_SESSION_EVENT";
		else if(ZOO_NOTWATCHING_EVENT == event) 
			return "ZOO_NOTWATCHING_EVENT";
		else
			return "unknown event";
	}

	string ConvertStateToMsg(int state)
	{
		if(ZOO_EXPIRED_SESSION_STATE == state)
			return "ZOO_EXPIRED_SESSION_STATE";
		else if(ZOO_AUTH_FAILED_STATE == state)
			return "ZOO_AUTH_FAILED_STATE";
		else if(ZOO_CONNECTING_STATE == state)
			return "ZOO_CONNECTING_STATE";
		else if(ZOO_ASSOCIATING_STATE == state)
			return "ZOO_ASSOCIATING_STATE";
		else if(ZOO_CONNECTED_STATE == state)
			return "ZOO_CONNECTED_STATE";
		else 
			return "unknown state";
	}

	void ZooKeeper::GlobalWatchFunc(zhandle_t*, int type, int state,const char* path, void* ctx)
	{
		ZooKeeper* pzk = (ZooKeeper*)ctx;
		
		if (!pzk->m_global_watcher_) return;
		
		if (type == ZOO_SESSION_EVENT) 
		{
			if (state == ZOO_EXPIRED_SESSION_STATE)
			{
				pzk->m_global_watcher_->OnSessionExpired();
			} 
			else if (state == ZOO_CONNECTED_STATE)
			{
				pzk->m_global_watcher_->OnConnected();
			} 
			else if (state == ZOO_CONNECTING_STATE) 
			{
				pzk->m_global_watcher_->OnConnecting();
			} 
			else 
			{
				LOG_ERROR( "don't know how to process other session event yet");
			}
		}
		else if (type == ZOO_CREATED_EVENT) 
		{
			pzk->m_global_watcher_->OnCreated(path);
		} 
		else if (type == ZOO_DELETED_EVENT) 
		{
			pzk->m_global_watcher_->OnDeleted(path);
		} 
		else if (type == ZOO_CHANGED_EVENT) 
		{
			pzk->m_global_watcher_->OnChanged(path);
		} 
		else if (type == ZOO_CHILD_EVENT) 
		{
			pzk->m_global_watcher_->OnChildChanged(path);
		}
		else if (type == ZOO_NOTWATCHING_EVENT) 
		{
			pzk->m_global_watcher_->OnNotWatching(path);
		} 
		else 
		{
			LOG_ERROR("unknown zookeeper event type");
		}
	}


	ZooKeeper::ZooKeeper()
	{
		m_logFile = NULL;
		m_zoo_handle_ = NULL;
		m_global_watcher_ = NULL;
	}

	ZooKeeper::~ZooKeeper()
	{

	}

	int ZooKeeper::StartService(const std::string& server_hosts, ZooWatcher* global_watcher, int timeout_ms, ZooLogLevel level)
	{
		std::string filename("log/zookeeper.log");
		m_logFile = fopen(filename.c_str(), "a+");
		if(!m_logFile)
		{
			return -1;
		}
		
		zoo_set_log_stream(m_logFile);
		zoo_set_debug_level(level);

		m_global_watcher_= global_watcher;
		m_zoo_handle_ = zookeeper_init(server_hosts.c_str(),GlobalWatchFunc,timeout_ms,NULL, this,0);

		if (NULL == m_zoo_handle_) 
		{
			return -1;
		}

	}

	void ZooKeeper::StopService()
	{
		if (m_zoo_handle_) 
		{
			int ret = zookeeper_close(m_zoo_handle_);
			if (ret != ZOK)
			{
				LOG_ERROR("StopService error ");
			}
		}
		if (m_logFile)
		{
			fclose(m_logFile);
		}
	}

	bool ZooKeeper::is_connected()
	{
		  return zoo_state(m_zoo_handle_) == ZOO_CONNECTED_STATE;
	}

	bool ZooKeeper::is_expired()
	{
		  return zoo_state(m_zoo_handle_) == ZOO_EXPIRED_SESSION_STATE;
	}

	bool ZooKeeper::Exists(const std::string& path, NodeStat* stat, bool watch )
	{
		int zoo_code = zoo_exists(m_zoo_handle_, path.c_str(), watch, stat);
		if (zoo_code == ZNONODE) 
		{
			return false;
		} 
		else if (zoo_code == ZOK)
		{
			return true;
		}
		else
		{
			LOG_ERROR("%s zoo_exists action error ,msg = %s" ,path.c_str(),ConvertCodeToMsg(zoo_code).c_str());
		}
	}

	std::string ZooKeeper::Create(const std::string& path,const std::string& value /*= std::string()*/,int flag /*= 0*/)
	{

	}






}