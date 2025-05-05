//============================================================================
// Name        : Lab3_Task1.cpp
// Author      : Alexander Lemutov
// Version     :
// Copyright   : Free copyright
// Description : Lab3, Task1, Server.
//============================================================================

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <winsock2.h>
#include <windows.h>

using boost::asio::ip::tcp;

int main() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0){
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return 1;
	}
	try {
		boost::asio::io_context io_context;

		tcp::acceptor acceptor(io_context, tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345));
		std::cout << "Server launched at 127.0.0.1:12345\nWaiting for connection...";

		for (;;) {
			tcp::socket socket(io_context);
			acceptor.accept(socket);
			std::cout << "Client connected: " << socket.remote_endpoint() << std::endl;

			boost::asio::streambuf buffer;

			boost::asio::read_until(socket, buffer, '\n');

			std::istream input_stream(&buffer);
			std::string message;
			std::getline(input_stream, message);

			std::cout << "Message received: " << message << std::endl;

			std::string response = "Received: " + message + "\n";
			boost::asio::write(socket, boost::asio::buffer(response));
			std::cout << "Answer sent. Closing connection.\n";
		}
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	WSACleanup();
	return 0;
}
