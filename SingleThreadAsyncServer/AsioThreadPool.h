//#pragma once
//#include "Singleton.h"
//
//#include <boost/asio.hpp>
//
//#include <chrono>
//#include <iostream>
//#include <vector>
//
//
//class AsioThreadPool : public Singleton<AsioThreadPool>
//{
//	friend class Singleton<AsioThreadPool>;
//public:
//	~AsioThreadPool();
//	AsioThreadPool& operator=(const AsioThreadPool&) = delete;
//	AsioThreadPool(const AsioThreadPool&) = delete;
//	boost::asio::io_context& GetIOServicePool();
//	void Stop();
//private:
//	AsioThreadPool(int size = std::thread::hardware_concurrency());
//
//
//	boost::asio::io_context _service;
//	std::unique_ptr<boost::asio::io_context::work> _work;
//	std::vector<std::thread> _threads;
//};
//
