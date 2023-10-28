#include "LogicSystem.h"


LogicSystem::LogicSystem() :_b_stop(false) {
	RegisterCallBack();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}
LogicSystem::~LogicSystem() {
	_b_stop = true;
	_cv.notify_one();
	_worker_thread.join();
}
void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg) {
	std::unique_lock<std::mutex> lock(_mutex);
	_msg_que.push(msg);
	if (_msg_que.size() == 1) {
		_cv.notify_one();
	}
}

void LogicSystem::DealMsg() {
	for (;;) {
		std::unique_lock<std::mutex> lock(_mutex);
		while (_msg_que.empty() && !_b_stop) {
			_cv.wait(lock);
		}

		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << "\n";
				auto call_back_iter = _funCallbacks.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _funCallbacks.end()) {
					_msg_que.pop();
					continue;
				}

				call_back_iter->second(msg_node->_session,
					msg_node->_recvnode->_msg_id,
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_total_len));
				_msg_que.pop();
			}
			break;
		}

		auto msg_node = _msg_que.front();
		std::cout << "recv msg id is" << msg_node->_recvnode->_msg_id << "\n";
		auto call_back_iter = _funCallbacks.find(msg_node->_recvnode->_msg_id);
		if (call_back_iter == _funCallbacks.end()) {
			_msg_que.pop();
			continue;
		}

		call_back_iter->second(msg_node->_session,
			msg_node->_recvnode->_msg_id,
			std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_total_len));
		_msg_que.pop();
	}
}

void LogicSystem::RegisterCallBack()
{
	_funCallbacks[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWorldCallBack, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg)
{
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg, root);
	std::cout << "receive msg id is" << root["id"].asInt() << " msg data is "
		<< root["data"].asString() << "\n";
	root["data"] = "server has receive msg, msg data is " + root["data"].asString();
	std::string retrun_str = root.toStyledString();

	session->Send(retrun_str, msg_id);
}

LogicSystem& LogicSystem::GetInstance()
{
	static LogicSystem instance;
	return instance;
}


