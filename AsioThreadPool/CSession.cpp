#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

CSession::CSession(boost::asio::io_context& io_context, CServer* server) :
	_socket(io_context), _server(server), _b_head_parse(false), _b_close(false), _strand(io_context.get_executor()) {
	//串行
	//_strand(io_context.get_executor());
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}
CSession::~CSession() {
	_socket.close();
	_b_close = true;
}
std::string& CSession::GetUuid() {
	return _uuid;
}
tcp::socket& CSession::GetSocket() {
	return _socket;
}
void CSession::Close() {
	_socket.close();
	_b_close = true;
}
void CSession::Start() {
	//读写变成串行
	memset(_data, 0, MAX_LENGHT);
	_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGHT),
		boost::asio::bind_executor(_strand, std::bind(&CSession::HandleRead,
			this, std::placeholders::_1, std::placeholders::_2, SessionSelf())
		));
}
std::shared_ptr<CSession> CSession::SessionSelf() {
	return shared_from_this();
}

void CSession::Send(char* msg, int max_len, short msg_id) {
	//发送的时候也是需要串行的
	//通过boost::asio::bind_exeutor(std::bind())来进行

	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_queue.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session :" << _uuid << " send que failed, size is" << MAX_SENDQUE << "\n";
		return;
	}

	_send_queue.push(std::make_shared<SendNode>(msg, max_len, msg_id));
	if (send_que_size > 0) {
		return;
	}
	auto& node = _send_queue.front();
	boost::asio::async_write(_socket, boost::asio::buffer(node->_data, node->_total_len), boost::asio::bind_executor(_strand, std::bind(
		&CSession::HandleWrite, this, std::placeholders::_1, SessionSelf())
	));
}

void CSession::Send(std::string msg, short msg_id) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_queue.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session :" << _uuid << " send que failed, size is" << MAX_SENDQUE << "\n";
		return;
	}

	_send_queue.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msg_id));
	if (send_que_size > 0) {
		return;
	}
	auto& node = _send_queue.front();
	boost::asio::async_write(_socket, boost::asio::buffer(node->_data, node->_total_len), boost::asio::bind_executor(_strand, std::bind(
		&CSession::HandleWrite, this, std::placeholders::_1, SessionSelf())
	));
}

void CSession::HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> session) {
	try
	{
		if (!ec) {
			std::lock_guard<std::mutex> lock(_send_lock);
			_send_queue.pop();
			if (!_send_queue.empty()) {
				auto& node = _send_queue.front();
				boost::asio::async_write(_socket, boost::asio::buffer(node->_data, node->_total_len), std::bind(
					&CSession::HandleWrite, this, std::placeholders::_1, session
				));
			}
		}
		else {
			std::cout << "Handle Write Failed, error is " << ec.what() << "\n";
			Close();
			//DOTO 从server列表中移除该csession
			_server->ClearSession(_uuid);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception code : " << e.what() << std::endl;
	}

}

void CSession::HandleRead(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> session) {
	if (!ec) {
		int copy_len = 0;
		while (bytes_transferred > 0) {
			if (!_b_head_parse) {
				if (bytes_transferred < HEAD_TOTAL_LEN) {//接收到比头部消息更短的数据
					memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data, bytes_transferred);
					_recv_head_node->_cur_len += bytes_transferred;
					memset(_data, 0, MAX_LENGHT);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGHT), boost::asio::bind_executor(_strand, std::bind(
						&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, session
					)));
					return;
				}
				//接收到的数据比头部长
				int remain_head = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
				memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data, remain_head);
				_recv_head_node->_cur_len += remain_head;
				bytes_transferred -= remain_head;
				copy_len += remain_head;

				//获取消息ID
				short msg_id = 0;
				memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
				//转成网络字节序
				msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
				std::cout << "msg id is " << msg_id << "\n";
				if (msg_id > MAX_LENGHT) {
					std::cout << "invalid msg_id is" << msg_id << "\n";
					//DOTO 将当前的Session断开
					_server->ClearSession(_uuid);
					return;
				}

				//获取长度
				short msg_len = 0;
				memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_MSG_LEN);
				msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
				std::cout << "msg_len is" << msg_len << "\n";
				if (msg_len > MAX_LENGHT) {
					std::cout << "invalid msg_len is " << msg_len << "\n";
					//DOTO 将当前的Session断开
					_server->ClearSession(_uuid);
					return;
				}

				_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
				if (bytes_transferred < msg_len) {//接收到不足 一条数据的消息 
					memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
					_recv_msg_node->_data += bytes_transferred;
					memset(_data, 0, MAX_LENGHT);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGHT), boost::asio::bind_executor(_strand, std::bind(
						&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, session
					)));
					_b_head_parse = true;
					return;
				}

				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
				_recv_msg_node->_cur_len += msg_len;
				bytes_transferred -= msg_len;
				copy_len += msg_len;
				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
				Json::Reader reader;
				Json::Value root;
				reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_cur_len), root);
				std::cout << "receive msg id is:" << root["id"].asInt() << " ,msg data is:" << root["data"].asString() << "\n";


				//DOTO发送回客户端
				LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(session, _recv_msg_node));

				//继续读取剩余的数据
				_b_head_parse = false;
				_recv_head_node->Clear();
				if (bytes_transferred <= 0) {
					memset(_data, 0, MAX_LENGHT);
					_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGHT), boost::asio::bind_executor(_strand, std::bind(
						&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, session
					)));
					return;
				}
				continue;
			}

			int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
			if (bytes_transferred < remain_msg) {
				memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
				_recv_msg_node->_cur_len += bytes_transferred;
				memset(_data, 0, MAX_LENGHT);
				_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGHT), boost::asio::bind_executor(_strand, std::bind(
					&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, session
				)));
				return;
			}
			memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
			_recv_msg_node->_cur_len += remain_msg;
			bytes_transferred -= remain_msg;
			copy_len += remain_msg;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';

			//DOTO 发送回客户端
			LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(session, _recv_msg_node));

			_b_head_parse = false;
			_recv_head_node->Clear();
			if (bytes_transferred <= 0) {
				memset(_data, 0, MAX_LENGHT);
				_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGHT), boost::asio::bind_executor(_strand, std::bind(
					&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, session
				)));
				return;
			}
			continue;
		}
	}
	else {
		std::cout << "Handle Read Faile, error is " << ec.what() << "\n";
		Close();
		//DOTO 从server列表中移除该csession
		_server->ClearSession(_uuid);
	}
}

void CSession::HandleReadHead(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> session) {
	if (!ec) {
		if (bytes_transferred < HEAD_TOTAL_LEN) {
			std::cout << "read head error \n";
			Close();
			_server->ClearSession(_uuid);
			return;
		}
		short msg_id = 0;
		memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
		msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
		std::cout << "read msg_id is " << msg_id << "\n";
		if (msg_id > MAX_LENGHT) {
			std::cout << "invalid msg_id is" << msg_id << "\n";
			Close();
			_server->ClearSession(_uuid);
			return;
		}
		short msg_len = 0;
		memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_MSG_LEN);
		msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
		std::cout << "read msg_len is " << msg_len << "\n";
		if (msg_len > MAX_LENGHT) {
			std::cout << "invalid msg_len is" << msg_id << "\n";
			Close();
			_server->ClearSession(_uuid);
			return;
		}

		_recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
		boost::asio::async_read(_socket, boost::asio::buffer(_recv_msg_node->_data, _recv_msg_node->_cur_len), std::bind(
			&CSession::HeadleReadMsg, this, std::placeholders::_1, std::placeholders::_2, session
		));
	}
	else {
		std::cout << "Handle Read Head Failed, error is" << ec.what() << "\n";
		Close();
		_server->ClearSession(_uuid);
	}
}
void CSession::HeadleReadMsg(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> session) {
	if (!ec) {
		_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
		std::cout << "read data is:" << _recv_msg_node->_data << "\n";

		_recv_head_node->Clear();
		boost::asio::async_read(_socket, boost::asio::buffer(_recv_head_node->_data, HEAD_TOTAL_LEN), std::bind(
			&CSession::HandleReadHead, this, std::placeholders::_1, std::placeholders::_2, session
		));
	}
	else {
		std::cout << "Heandle Read Msg failed, error is " << ec.what() << "\n";
		Close();
		_server->ClearSession(_uuid);
	}
}
