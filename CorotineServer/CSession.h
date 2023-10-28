#pragma once

#include "MsgNode.h"

#include <boost/asio.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>


using boost::asio::ip::tcp;

class CServer;

class CSession:public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& io_context, CServer* server);
	~CSession();

	tcp::socket& GetSocket();
	std::string& GetUuid();
	void Close();
	void Start();
	void Send(const char* msg, short max_length, short msg_id);
	void Send(std::string msg, short msg_id);
private:
	void HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> session);
	boost::asio::io_context& _io_context;
	tcp::socket _socket;
	CServer* _server;
	std::string _uuid;
	bool _b_close;
	std::mutex _send_lock;
	std::queue<std::shared_ptr<SendNode>> _send_que;
	std::shared_ptr<RecvNode> _recv_msg_node;
	std::shared_ptr<MsgNode> _recv_head_node;
};

class LogicNode {
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);

	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recvnode;
};

