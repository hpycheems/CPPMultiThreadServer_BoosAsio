#pragma once
#include "CSession.h"

#include <map>

class CServer {
public:
	CServer(boost::asio::io_context& io_context, short port);
	void ClearSession(std::string uuid);
private:
	void StartAccept();
	void HandleAccept(const boost::system::error_code& ec, std::shared_ptr<CSession> newSession);
	boost::asio::io_context& _io_context;
	tcp::acceptor _acceptor;
	short _port;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};