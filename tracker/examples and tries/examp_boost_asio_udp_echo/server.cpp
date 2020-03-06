#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <set>

using boost::asio::ip::udp;

enum {
    max_length = 1024
};

void server(boost::asio::io_context &io_context, unsigned short port) {
    udp::socket sock(io_context, udp::endpoint(udp::v4(), port));
    for (;;) {
        char data[max_length];
        udp::endpoint sender_endpoint;
        size_t length = sock.receive_from(boost::asio::buffer(data, max_length), sender_endpoint);
        sock.send_to(boost::asio::buffer(data, length), sender_endpoint);
    }
}


int main(int argc, char *argv[]) {

    boost::asio::io_context io_context;
    server(io_context, 8001);

    return 0;
}