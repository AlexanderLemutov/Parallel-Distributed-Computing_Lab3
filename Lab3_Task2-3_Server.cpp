//============================================================================
// Name        : Lab3_Task2_Server.cpp
// Author      : Alexander Lemutov
// Version     :
// Copyright   : Free copyright
// Description : Lab3, Task2, Server.
//============================================================================

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <windows.h>
#include <chrono>

using boost::asio::ip::tcp;

uint64_t factorial(int num){
	if (num == 0){
		return 1;
	}
	if (num == 1) {
		return 1;
	}
	return num*factorial(num-1);
}

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
		std::cout << "Server launched at 127.0.0.1:12345\nWaiting for connection...\n";

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

			std::istringstream iss(message);
			std::string command;
			int number;

			iss >> command >> number;

			if (command == "number") {
				boost::asio::post([number, socket = std::move(socket)]() mutable {
					try {
						uint64_t result = factorial(number);

						std::string response = "Client entered: " + std::to_string(number) + "; " + "Server return: " + std::to_string(result) + "\n";
						boost::asio::write(socket, boost::asio::buffer(response));
						std::cout << "Answer sent. Closing connection.\n";
					} catch (std::exception& e) {
						std::cerr << "Error: " << e.what() << std::endl;
					}

				});
			} else if (command == "timer") {
				boost::asio::steady_timer timer(io_context, std::chrono::seconds(number));
				timer.wait();
				std::string response = "Timer went off!\n";
				boost::asio::write(socket, boost::asio::buffer(response));
				std::cout << "Answer sent. Closing connection.\n";
			}
		}
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	WSACleanup();
	return 0;
}
