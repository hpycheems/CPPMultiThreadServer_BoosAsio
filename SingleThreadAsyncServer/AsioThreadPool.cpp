//#include "AsioThreadPool.h"
//
//AsioThreadPool::AsioThreadPool(int size):
//	_work(new boost::asio::io_context::work(_service)){
//	for (int i = 0; i < size; ++i) {
//		_threads.emplace_back([this] {
//			_service.run();
//			});
//	}
//}
//
//AsioThreadPool::~AsioThreadPool()
//{
//	std::cout << "AsioThreadPool destruct\n";
//}
//
//boost::asio::io_context& AsioThreadPool::GetIOServicePool() {
//	_work.reset();
//}
//
//void AsioThreadPool::Stop() {
//	for (auto& t : _threads) {
//		t.join();
//	}
//}