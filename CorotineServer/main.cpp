#include "CServer.h"
#include "AsioIOServicePool.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <csignal>

int main() {

	try
	{
		auto& pool = AsioIOServicePool::GetInstance();
		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT);
		signals.async_wait([&](auto, auto) {
			io_context.stop();
			pool.Stop();
			});

		CServer s(io_context, 8888);
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception :" << e.what() << "\n";
	}

	return 0;
}
