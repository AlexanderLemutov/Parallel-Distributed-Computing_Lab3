//============================================================================
// Name        : Lab3_Task4_Server.cpp
// Author      : Alexander Lemutov
// Version     :
// Copyright   : Free copyright
// Description : Lab3, Task 4, Server.
//============================================================================
#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <boost/asio/strand.hpp>
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <memory>
#include <sstream>
#include <chrono>

using boost::asio::ip::tcp;
using namespace std::chrono_literals;

std::vector<std::string> log;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, boost::asio::strand<boost::asio::io_context::executor_type> strand):socket_(std::move(socket)), strand_(strand) {}
    void start() {
        read();
    }
private:
    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::string data_;

    void read() {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(data_), '\n',
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string message = data_.substr(0, length - 1);
                    data_.erase(0, length);

                    int n = std::stoi(message);
                    boost::asio::post(socket_.get_executor(),
                        [this, self, n]() {
                            std::string result = "factorial(" + std::to_string(n) + ") = " + std::to_string(factorial(n)) + "\n";

                            boost::asio::post(strand_, [result]() {
                                log.push_back(result);
                            });

                            write(result);
                        });
                }
            });
    }

    void write(const std::string& response) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    read();
                }
            });
    }

    uint64_t factorial(int n) {
        if (n == 0 || n == 1){
        	return 1;
        } else {
        	uint64_t result = 1;
        	for (int i = 2; i <= n; ++i)
        		result *= i;
        	return result;
        }
    }
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port, int num_threads)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          strand_(boost::asio::make_strand(io_context)) {
        do_accept();
    }

private:
    tcp::acceptor acceptor_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    void do_accept() {
        acceptor_.async_accept(
            boost::asio::make_strand(acceptor_.get_executor().context()),
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), strand_)->start();
                }
                do_accept();
            });
    }
};

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0){
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return 1;
	}
    try {
        if (argc != 2) {
            std::cerr << "Usage: server <threads>\n";
            return 1;
        }

        int num_threads = std::atoi(argv[1]);
        boost::asio::io_context io_context;

        Server s(io_context, 12345, num_threads);

        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&io_context]() { io_context.run(); });
        }
        for (auto &t: threads) t.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    WSACleanup();
    return 0;
}
