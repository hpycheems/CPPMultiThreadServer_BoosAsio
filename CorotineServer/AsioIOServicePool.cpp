#include "AsioIOServicePool.h"
#include <iostream>

AsioIOServicePool::AsioIOServicePool(std::size_t thread_num)
	:_ioService(thread_num), _works(thread_num), _next_IOService(0)
{
	for (size_t i = 0; i < thread_num; ++i)
	{
		_works[i] = std::unique_ptr<Work>(new Work(_ioService[i]));
	}

	for (size_t i = 0; i < _ioService.size(); i++)
	{
		_threads.emplace_back([this,i]() {
			_ioService[i].run();
			});
	}
}

AsioIOServicePool::~AsioIOServicePool() {
	std::cout << "AsioIOService destruct\n";
}

AsioIOServicePool& AsioIOServicePool::GetInstance() {
	static AsioIOServicePool instance(1);
	return instance;
}

boost::asio::io_context& AsioIOServicePool::GetIOService() {
	auto& service = _ioService[_next_IOService];
	if (_next_IOService == _ioService.size()) {
		_next_IOService = 0;
	}
	return service;
}

void AsioIOServicePool::Stop() {
	for (auto& work : _works) {
		work.reset();
	}

	for (auto& t : _threads) {
		t.join();
	}
}