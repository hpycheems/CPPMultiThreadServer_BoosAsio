#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include <chrono>
#include <thread>
#include <iostream>

using boost::asio::ip::tcp;
const int MAX_LENGHT = 1024 * 2;
const int HEAD_TOTAL = 4;
const int HEAD_LENGHT = 2;

std::vector<std::thread> vec_threads;

int main() {
	/*Json::Value root;
	root["id"] = 1001;
	root["data"] = "hello world";
	std::string request = root.toStyledString();
	std::cout << "request is " << request << std::endl;
	Json::Value root2;
	Json::Reader reader;
	reader.parse(request, root2);
	std::cout << "msg id is " << root2["id"] << " msg is " << root2["data"] << std::endl;*/

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 100; i++)
	{
		vec_threads.emplace_back([]() {
			try
			{
				boost::asio::io_context ioc;
				tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
				tcp::socket sock(ioc);
				boost::system::error_code ec;
				sock.connect(ep, ec);
				if (ec) {
					std::cout << "socket connect failed, error is" << ec.what() << "\n";
					return 0;
				}

				int i = 0;
				while (i < 500) {
					//发送数据
					Json::Value root;
					root["id"] = 1001;
					root["data"] = "hello world";
					std::string request = root.toStyledString();
					std::size_t request_length = request.length();
					std::cout << request_length << "\n";
					char send_data[MAX_LENGHT] = { 0 };
					int msgid = 1001;
					int msgid_host = boost::asio::detail::socket_ops::host_to_network_short(msgid);
					memcpy(send_data, &msgid_host, 2);
					int request_host_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);
					memcpy(send_data + 2, &request_host_length, 2);
					memcpy(send_data + 4, request.c_str(), request_length);
					boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 4));
					std::cout << "begin to recvive ..\n";

					char reply_head[HEAD_TOTAL];
					std::size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL));

					msgid = 0;
					memcpy(&msgid, reply_head, HEAD_LENGHT);
					short msglen = 0;
					memcpy(&msglen, reply_head + 2, HEAD_LENGHT);
					msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
					msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
					char msg[MAX_LENGHT] = { 0 };
					std::size_t msg_lenght = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
					Json::Reader reader;
					reader.parse(std::string(msg, msg_lenght), root);
					std::cout << "msg id is" << root["id"] << " msg is " << root["data"] << "\n";
					++i;
				}

			}
			catch (const std::exception& e)
			{
				std::cout << "Exception:" << e.what() << "\n";
			}
			});
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	for (auto& t : vec_threads) {
		t.join();
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
	std::cout << "Time spent :" << duration.count() << "seconds." << "\n";

	/*
	try
	{
		boost::asio::io_context ioc;
		tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
		tcp::socket sock(ioc);
		boost::system::error_code ec;
		sock.connect(ep, ec);
		if (ec) {
			std::cout << "socket connect failed, error is" << ec.what() << "\n";
			return 0;
		}

		//发送数据
		Json::Value root;
		root["id"] = 1001;
		root["data"] = "hello world";
		std::string request = root.toStyledString();
		std::size_t request_length = request.length();
		std::cout << request_length << "\n";
		char send_data[MAX_LENGHT] = { 0 };
		int msgid = 1001;
		int msgid_host = boost::asio::detail::socket_ops::host_to_network_short(msgid);
		memcpy(send_data, &msgid_host, 2);
		int request_host_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);
		memcpy(send_data + 2, &request_host_length, 2);
		memcpy(send_data + 4, request.c_str(), request_length);
		boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 4));
		std::cout << "begin to recvive ..\n";

		Json::Reader reader_;
		Json::Value value;
		reader_.parse(request, value);
		std::cout << "id:" << value["id"] << ",data" << value["data"] << "\n";


		char reply_head[HEAD_TOTAL];
		std::size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL));

		msgid = 0;
		memcpy(&msgid, reply_head, HEAD_LENGHT);
		short msglen = 0;
		memcpy(&msglen, reply_head + 2, HEAD_LENGHT);
		msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
		msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
		char msg[MAX_LENGHT] = { 0 };
		std::size_t msg_lenght = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
		Json::Reader reader;
		reader.parse(std::string(msg, msg_lenght), root);
		std::cout << "msg id is" << root["id"] << " msg is " << root["data"] << "\n";


	}
	catch (const std::exception& e)
	{
		std::cout << "error is:" << e.what() << "\n";
	}
	*/
	system("pause");
	return 0;
}