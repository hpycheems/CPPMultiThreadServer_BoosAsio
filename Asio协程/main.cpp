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
* Asio携程的使用
*	使用awaitable<void>声明一个函数之后，该函数就变为可等待的函数了
*	co_spawn 表示启动一个协程，参数分别为调度器（io_context也是调度器），执行函数，以及启动方式
*	detached表示将协程对象分离出来，这种启动方式可以启动多个协程，它们都是独立的，如何调度取决于调度器，
*	用户在感知上更像是线程调度的模式，类似于并发运行，其实底层是串行的
*   co_await 等待函数执行，如果该函数没有触发，则挂起协程。
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
		//detached 将
		co_spawn(io_context, listener(), detached);
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception is " << e.what() << "\n";
	}

	return 0;
}