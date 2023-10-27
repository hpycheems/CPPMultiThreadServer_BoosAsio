#include "CServer.h"
#include "AsioThreadPool.h"

#include <iostream>

/*
* 逻辑层一般使用单线程
* 使用单例模式
*
* 经验：在头文件中 尽量用class Class，先声明出要使用的类，除非该类必须是完整的，在cpp文件中在把它包含进来
*       静态成员必须在类外表 定义
*
* 多线程：
*   启动多个线程：每个线程跑一个io_context
*   启动多个线程：线程之间用一个io_context
* 
* 
*	频繁的加锁效率还不如多线程
*/

//优雅的退出服务器 c版本
#include <csignal>
//bool bstop = false;
//std::condition_variable cond_quit;
//std::mutex mutex_quit;
//void sig_handler(int sig) {
//    if (sig == SIGINT || sig == SIGTERM) {
//        std::unique_lock<std::mutex> lock_quit(mutex_quit);
//        bstop = true;
//        lock_quit.unlock();
//        cond_quit.notify_one();
//    }
//}

bool bstop = false;
std::condition_variable cond_quit;
std::mutex _mutex;

int main() {
	try
	{
		//C版本退出

		//std::thread t([&io_context] {
		//    CServer s(io_context, 8888);
		//    });

		//signal(SIGINT, sig_handler);
		//signal(SIGTERM, sig_handler);
		//while (!bstop) {
		//    std::unique_lock<std::mutex> lock_quit(mutex_quit);
		//    cond_quit.wait(lock_quit);
		//}
		//io_context.stop();
		//t.join();

		boost::asio::io_context io_context;
		//使用AsioIOServicePool初始化
		auto pool = AsioThreadPool::GetInstance();
		//boost版本退出
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

		signals.async_wait([&io_context, &pool](auto, auto) {
			io_context.stop(); 
			pool->Stop();
			std::unique_lock<std::mutex> lock(_mutex);
			bstop = true;
			cond_quit.notify_all();
		});

		CServer s(pool->GetIOServicePool(), 8888);
		{
			std::unique_lock<std::mutex> lock(_mutex);
			while (!bstop) {
				cond_quit.wait(lock);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}