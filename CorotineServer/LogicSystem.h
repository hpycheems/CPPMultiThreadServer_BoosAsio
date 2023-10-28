#pragma once

#include "CSession.h"
#include "const.h"

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include <map>
#include <queue>
#include <thread>
#include <functional>
#include <memory>

typedef std::function<void(std::shared_ptr<CSession>,
	const short& msg_id, const std::string& msg_data)> FunCallBack;

class LogicSystem
{
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);
	static LogicSystem& GetInstance();
	LogicSystem& operator=(const LogicSystem&) = delete;
	LogicSystem(const LogicSystem&) = delete;
private:
	LogicSystem();
	void DealMsg();
	void RegisterCallBack();
	void HelloWorldCallBack(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg);

	std::thread _worker_thread;
	std::queue<std::shared_ptr<LogicNode>> _msg_que;
	std::mutex _mutex;
	std::condition_variable _cv;
	bool _b_stop;
	std::map<short, FunCallBack> _funCallbacks;
};