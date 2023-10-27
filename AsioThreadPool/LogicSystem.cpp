#include "LogicSystem.h"
#include "CSession.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

LogicSystem::LogicSystem() :_b_stop(false) {
	RegisterCallback();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem()
{
	_b_stop = true;
	_cv.notify_one();
	_worker_thread.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> _lock(_mutex);
	_msg_que.push(msg);
	if (_msg_que.size() == 1) {
		_lock.unlock();
		_cv.notify_one();
	}
}

void LogicSystem::RegisterCallback() {
	_func_callbacks[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWorldCallback, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::DealMsg() {
	while (true) {
		std::unique_lock<std::mutex> _lock(_mutex);
		while (_msg_que.empty() && !_b_stop)
			_cv.wait(_lock);
		if (_b_stop) {
			while (!_msg_que.empty()) {
				auto msg_node = _msg_que.front();
				std::cout << "receive msg_id is" << msg_node->_recvNode->_msg_id << "\n";
				auto call_back_iter = _func_callbacks.find(msg_node->_recvNode->_msg_id);
				if (call_back_iter == _func_callbacks.end()) {
					_msg_que.pop();
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvNode->_msg_id, std::string(msg_node->_recvNode->_data,
					msg_node->_recvNode->_cur_len));
				_msg_que.pop();
			}
			break;
		}

		auto msg_node = _msg_que.front();
		std::cout << "receive msg_id is" << msg_node->_recvNode->_msg_id << "\n";
		auto call_back_iter = _func_callbacks.find(msg_node->_recvNode->_msg_id);
		if (call_back_iter == _func_callbacks.end()) {
			_msg_que.pop();
			continue;
		}
		call_back_iter->second(msg_node->_session, msg_node->_recvNode->_msg_id,
			std::string(msg_node->_recvNode->_data, msg_node->_recvNode->_cur_len));
		_msg_que.pop();
	}
}

void LogicSystem::HelloWorldCallback(std::shared_ptr<CSession> session, const short msg_id, const std::string msg) {
	Json::Reader read;
	Json::Value value;
	read.parse(msg, value);
	std::cout << "receive msg id is:" << value["id"].asInt() << " ,msg data is:" << value["data"].asString() << "\n";
	value["data"] = "server has received msg, msg data is " + value["data"].asString();
	std::string return_str = value.toStyledString();
	session->Send(return_str, value["id"].asInt());
}