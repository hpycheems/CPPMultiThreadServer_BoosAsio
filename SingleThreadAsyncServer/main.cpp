#include "CServer.h"
#include "AsioIOServicePool.h"

#include <iostream>

/*
* �߼���һ��ʹ�õ��߳�
* ʹ�õ���ģʽ
*
* ���飺��ͷ�ļ��� ������class Class����������Ҫʹ�õ��࣬���Ǹ�������������ģ���cpp�ļ����ڰ�����������
*       ��̬��Ա����������� ����
*
* ���̣߳�
*   ��������̣߳�ÿ���߳���һ��io_context
*   ��������̣߳��߳�֮����һ��io_context
*/

//���ŵ��˳������� c�汾
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

int main() {
	try
	{
		//C�汾�˳�

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
		//ʹ��AsioIOServicePool��ʼ��
		auto pool = AsioIOServicePool::GetInstance();
		//boost�汾�˳�
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

		signals.async_wait([&io_context, &pool](auto, auto) {
			io_context.stop(); 
			pool->Stop();
		});

		CServer s(io_context, 8888);
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	return 0;
}