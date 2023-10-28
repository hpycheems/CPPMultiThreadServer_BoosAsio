#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

awaitable<void> echo(tcp::socket socket) {
	try
	{
		char data[1024];
		for (;;) {
			std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
			co_await boost::asio::async_write(socket, boost::asio::buffer(data, n), use_awaitable);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "exception is" << e.what() << "\n";
	}
}

awaitable<void> listener() {
	auto executor = co_await this_coro::executor;
	tcp::acceptor acceptor(executor, { tcp::v4(), 8888 });
	for (;;) {
		tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
		co_spawn(executor, echo(std::move(socket)), detached);
	}
}

/*
* AsioЯ�̵�ʹ��
*	ʹ��awaitable<void>����һ������֮�󣬸ú����ͱ�Ϊ�ɵȴ��ĺ�����
*	co_spawn ��ʾ����һ��Э�̣������ֱ�Ϊ��������io_contextҲ�ǵ���������ִ�к������Լ�������ʽ
*	detached��ʾ��Э�̶���������������������ʽ�����������Э�̣����Ƕ��Ƕ����ģ���ε���ȡ���ڵ�������
*	�û��ڸ�֪�ϸ������̵߳��ȵ�ģʽ�������ڲ������У���ʵ�ײ��Ǵ��е�
*   co_await �ȴ�����ִ�У�����ú���û�д����������Э�̡�
* 
*/

int main() {
	
	try
	{
		boost::asio::io_context io_context;
		boost::asio::signal_set signals(io_context, SIGINT);
		signals.async_wait([&](auto, auto) {
			io_context.stop();
			});
		//detached ��
		co_spawn(io_context, listener(), detached);
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception is " << e.what() << "\n";
	}

	return 0;
}