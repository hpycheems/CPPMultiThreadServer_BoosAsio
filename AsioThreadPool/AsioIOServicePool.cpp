#include "AsioIOServicePool.h"

AsioIOServicePool::~AsioIOServicePool()
{
	std::cout << "AsioIOServicePool destruce\n";
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& service = _ioService[_nextIOService++];
	if (_nextIOService == _ioService.size()) {
		_nextIOService = 0;
	}
	return service;
}

void AsioIOServicePool::Stop()
{
	for (auto& work : _works) {
		work.reset();//把work智能指针析构
	}

	//等待线程退出
	for (auto& t : _threads) {
		t.join();
	}
}

AsioIOServicePool::AsioIOServicePool(std::size_t size)//size是建议开辟的线程数量
	:_ioService(size),_nextIOService(0),_works(size)
{
	std::cout << "open:" << size << "thread\n";
	for (size_t i = 0; i < size; ++i)
	{
		_works[i] = std::unique_ptr<Work>(new Work(_ioService[i]));
	}

	//创建线程，每个线程内部启动ioservic
	for (size_t i = 0; i < _ioService.size(); ++i)
	{
		_threads.emplace_back([this, i]() {
			_ioService[i].run();
			});
	}
}