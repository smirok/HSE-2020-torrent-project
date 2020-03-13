#include <iostream>
#include <boost/asio.hpp>
#include "server.hpp"


int main(int argc, char *argv[]) {

    boost::asio::io_context io_context;
    Server server(io_context, 8001);
    server.start();

    return 0;
}