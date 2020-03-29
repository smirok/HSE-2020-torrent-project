#include <iostream>
#include <boost/asio.hpp>
#include "server.hpp"

enum class Kek : uint64_t{
    one, two
};

int f(int a) {
    std::cout << a;
    return 1;
}

int main(/*int argc, char *argv[]*/) {
    boost::asio::io_context io_context;
    UDP_server::Server server(io_context, 8001);
    server.start();
    return 0;
}