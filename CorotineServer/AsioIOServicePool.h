#pragma once

#include <boost/asio.hpp>

#include <thread>
#include <memory>
#include <vector>

class AsioIOServicePool
{
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	boost::asio::io_context& GetIOService();
	void Stop();
	static AsioIOServicePool& GetInstance();
private:
	AsioIOServicePool(std::size_t thread_num = std::thread::hardware_concurrency());
	std::vector<IOService> _ioService;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	int _next_IOService;
};

