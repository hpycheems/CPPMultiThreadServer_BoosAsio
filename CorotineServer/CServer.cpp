#include "CServer.h"
#include "CSession.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& io_context, short port)
	:_io_context(io_context), _port(port), _acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
	std::cout << "Server Start Listening Port :" << port << "\n";
	StartAcceptor();
}

CServer::~CServer()
{
	std::cout << "Servcer destruct success, listen on port" << _port << "\n";
}

void CServer::HandleAcceptor(std::shared_ptr<CSession> session, const boost::system::error_code ec)
{
	try
	{
		session->Start();
		std::lock_guard<std::mutex> lock(_mutex);
		_sessions.insert(std::make_pair(session->GetUuid(), session));
	}
	catch (const std::exception& e)
	{
		std::cout << "exception Handle Acceptor " << e.what() << "\n";
	}

	StartAcceptor();//切记不要忘了继续监听后来的连接
}

void CServer::StartAcceptor()
{
	auto& io_context = AsioIOServicePool::GetInstance().GetIOService();
	auto new_Session = std::make_shared<CSession>(io_context, this);
	_acceptor.async_accept(new_Session->GetSocket(),
		std::bind(&CServer::HandleAcceptor, this, new_Session, std::placeholders::_1));
}

void CServer::ClearSesseion(std::string uuid)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_sessions.erase(uuid);
}