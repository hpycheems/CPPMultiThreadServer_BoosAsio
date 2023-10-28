#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

const int MAX_LENGHT = 1024;

int main() {
	try
	{
		boost::asio::io_context io_context;
		tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
		boost::system::error_code ec = boost::asio::error::host_not_found;
		tcp::socket socket(io_context);
		socket.connect(ep, ec);
		if (ec) {
			std::cout << "socket connect failed " << ec.value() 
				<< " error msg is " << ec.what() << "\n";
		}

		std::cout << "Enter msg:\n";
		char request[MAX_LENGHT];
		std::cin.getline(request, MAX_LENGHT);
		std::size_t request_length = std::strlen(request);
		boost::asio::write(socket, boost::asio::buffer(request, request_length));

		char reply[MAX_LENGHT];
		boost::asio::read(socket, boost::asio::buffer(reply, request_length));
		reply[request_length] = '\0';
		std::cout << "receive data :" << reply << "\n";
	}
	catch (const std::exception& e)
	{
		std::cout << "exception is " << e.what() << "\n";
	}

	system("pause");
	return 0;
}