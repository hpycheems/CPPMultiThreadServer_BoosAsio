#pragma once
#include "Singleton.h"

#include <memory>
#include <mutex>
#include <iostream>
#include <queue>
#include <thread>
#include <map>
#include <functional>

class CSession;
class LogicNode;

typedef std::function<void(std::shared_ptr<CSession>, const short msg_id, const std::string msg_data)> FuncCallBack;

class LogicSystem : public Singleton<LogicSystem> {
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
	LogicSystem();
	void DealMsg();
	void RegisterCallback();
	void HelloWorldCallback(std::shared_ptr<CSession> session, const short msg_id, const std::string msg);

	std::mutex _mutex;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::condition_variable _cv;
	std::thread _worker_thread;
	bool _b_stop;
	//id and callback
	std::map<short, FuncCallBack> _func_callbacks;
};