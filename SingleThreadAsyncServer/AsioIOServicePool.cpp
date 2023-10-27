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
		work.reset();//��work����ָ������
	}

	//�ȴ��߳��˳�
	for (auto& t : _threads) {
		t.join();
	}
}

AsioIOServicePool::AsioIOServicePool(std::size_t size)//size�ǽ��鿪�ٵ��߳�����
	:_ioService(size),_nextIOService(0),_works(size)
{
	std::cout << "open:" << size << "thread\n";
	for (size_t i = 0; i < size; ++i)
	{
		_works[i] = std::unique_ptr<Work>(new Work(_ioService[i]));
	}

	//�����̣߳�ÿ���߳��ڲ�����ioservic
	for (size_t i = 0; i < _ioService.size(); ++i)
	{
		_threads.emplace_back([this, i]() {
			_ioService[i].run();
			});
	}
}