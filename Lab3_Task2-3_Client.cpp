//============================================================================
// Name        : Lab3_Task2_Client.cpp
// Author      : Alexander Lemutov
// Version     :
// Copyright   : Free copyright
// Description : Lab3, Task2, Client.
//============================================================================

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <winsock2.h>
#include <windows.h>


using boost::asio::ip::tcp;

int main(){
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0){
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return 1;
	}
	try {
		boost::asio::io_context io_context;
		tcp::socket socket(io_context);

		tcp::endpoint endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
		socket.connect(endpoint);

		std::cout << "Connected to server.\n";

		std::cout << "Write a text: " << std::endl;
		std::string message;
		std::getline(std::cin, message);
		message = message + "\n";
		std::cout << "Message sent: " + message;
		boost::asio::write(socket, boost::asio::buffer(message));

		boost::asio::streambuf buffer;
		boost::asio::read_until(socket, buffer, "\n");

		std::istream input_stream(&buffer);
		std::string response;
		std::getline(input_stream, response);
		std::cout << "Answer from server: " + response << std::endl;
	} catch (std::exception &e){
		std::cerr << "Error of client: " << e.what() << std::endl;
	}
	WSACleanup();
}
