#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

using boost::asio::ip::tcp;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::strand;
using boost::asio::io_context;
namespace this_coro = boost::asio::this_coro;

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
	:_socket(io_context), _server(server), _io_context(io_context), _b_close(false) {
	boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

tcp::socket& CSession::GetSocket() {
	return _socket;
}
std::string& CSession::GetUuid() {
	return _uuid;
}
void CSession::Start() {
	auto shared_this = shared_from_this();
	boost::asio::co_spawn(_io_context, [=]()->boost::asio::awaitable<void> {
		try
		{
			for (; !_b_close;) {
				_recv_head_node->Clear();
				std::size_t n = co_await boost::asio::async_read(_socket,
					boost::asio::buffer(_recv_head_node->_data, HEAD_TOTAL_LEN), use_awaitable);
				if (n == 0) {
					std::cout << "recvive peer closed\n";
					Close();
					_server->ClearSesseion(_uuid);
					co_return;
				}

				short msg_id = 0;
				memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);	
				msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
				std::cout << "msg_id is :" << msg_id << "\n";
				if (msg_id > MAX_LENGHT) {
					std::cout << "invalid msg id is:" << msg_id << "\n";
					Close();
					_server->ClearSesseion(_uuid);
					co_return;
				}

				short msg_length = 0;
				memcpy(&msg_length, _recv_head_node->_data + HEAD_ID_LEN, HEAD_MSG_LEN);
				msg_length = boost::asio::detail::socket_ops::network_to_host_short(msg_length);
				std::cout << "msg_length is :" << msg_length << "\n";
				if (msg_length > MAX_LENGHT) {
					std::cout << "invalid msg len is:" << msg_length << "\n";
					Close();
					_server->ClearSesseion(_uuid);
					co_return;
				}

				_recv_msg_node = std::make_shared<RecvNode>(msg_length, msg_id);
				n == co_await boost::asio::async_read(_socket,
					boost::asio::buffer(_recv_msg_node->_data, _recv_msg_node->_total_len), use_awaitable);

				if (n == 0) {
					std::cout << "recvive peer closed \n";
					Close();
					_server->ClearSesseion(_uuid);
					co_return;
				}

				_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
				std::cout << "receive data is" << _recv_msg_node->_data << "\n";
				//投递给逻辑线程
				LogicSystem::GetInstance().PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			}
		}
		catch (const std::exception& e)
		{
			std::cout << "Exception Start：" << e.what() << " error code:" << "\n";
			Close();
			_server->ClearSesseion(_uuid);
		}
		}, detached);
}
void CSession::Send(const char* msg, short max_length, short msg_id)
{
	std::unique_lock<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session :" << _uuid << " send que full" << MAX_SENDQUE << "\n";
		return;
	}
	_send_que.push(std::make_shared<SendNode>(msg, max_length, msg_id));
	if (send_que_size > 0) {
		return;
	}
	auto msgnode = _send_que.front();
	lock.unlock();//当用完共享元素之后即可解锁
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}
void CSession::Send(std::string msg, short msg_id)
{
	Send(msg.c_str(), msg.length(), msg_id);
}

void CSession::HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> session) {
	try
	{
		if (!ec) {
			std::unique_lock<std::mutex> lock(_send_lock);
			_send_que.pop();
			if (!_send_que.empty()) {
				auto msgnode = _send_que.front();
				lock.unlock();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&CSession::HandleWrite, this, std::placeholders::_1, session)
				);
			}
		}
		else {
			std::cout << "handle write exception, error is" << ec.what() << "\n";
			Close();
			_server->ClearSesseion(_uuid);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception Handle Write" << e.what() << "\n";
		Close();
		_server->ClearSesseion(_uuid);
	}
}

CSession::~CSession() {
	try
	{
		std::cout << "CSession destuced \n";
		Close();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception ~CSession" << e.what() << "\n";
	}
}

void CSession::Close() {
	_socket.close();
	_b_close = true;
}

LogicNode::LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recvnode) :
	_session(session), _recvnode(recvnode)
{

}
