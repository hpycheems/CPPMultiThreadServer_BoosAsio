#include "AsioThreadPool.h"
#include <iostream>

AsioThreadPool::AsioThreadPool(int thread_Num) :
	_work(new boost::asio::io_context::work(_service)){//����̹߳���һ�� io_context �ڹ��캯���д���������

	std::cout << thread_Num << "\n";

	for (int i = 0; i < thread_Num; ++i) {//�����߳�
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
	for (auto& t : _threads) {//�ȴ������߳̽���
		t.join();
	}
	std::cout << "AsioThreadPool Stop \n";
}