#include "AsioThreadPool.h"
#include <iostream>

AsioThreadPool::AsioThreadPool(int thread_Num) :
	_work(new boost::asio::io_context::work(_service)){//多个线程共用一个 io_context 在构造函数中创建上下文

	std::cout << thread_Num << "\n";

	for (int i = 0; i < thread_Num; ++i) {//开辟线程
		_threads.emplace_back([this] {
			_service.run();
			});
	}
}

AsioThreadPool::~AsioThreadPool()
{
	std::cout << "AsioThreadPool destruct\n";
}

boost::asio::io_context& AsioThreadPool::GetIOServicePool() {
	return _service;
}

void AsioThreadPool::Stop() {
	_service.stop();
	_work.reset();
	for (auto& t : _threads) {//等待工作线程结束
		t.join();
	}
	std::cout << "AsioThreadPool Stop \n";
}