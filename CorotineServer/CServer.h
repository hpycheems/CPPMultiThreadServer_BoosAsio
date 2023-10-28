#pragma once

#include <boost/asio.hpp>

#include <memory>
#include <mutex>
#include <map>
#include <iostream>

using boost::asio::ip::tcp;

class CSession;

class CServer
{
public:
	CServer(boost::asio::io_context& io_context, short port);
	~CServer();
	void ClearSesseion(std::string uuid);
private:
	void StartAcceptor();
	void HandleAcceptor(std::shared_ptr<CSession>, const boost::system::error_code);
	boost::asio::io_context& _io_context;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
	std::mutex _mutex;
};

