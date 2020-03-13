#include "server.hpp"

#include <utility>

Peer::Peer(udp::endpoint endpoint) :
        endpoint_(std::move(endpoint)) {}

uint32_t Peer::ip() const noexcept {
    return endpoint_.address().to_v4().to_ulong();
}

uint16_t Peer::port() const noexcept {
    return endpoint_.port();
}

udp::endpoint Peer::ep() const noexcept {
    return endpoint_;
}

bool Peer::operator<(const Peer &other) const noexcept {
    if (ip() != other.ip())
        return ip() < other.ip();
    else
        return port() < other.port(); // надо ли??
}

Torrent::Torrent(std::string  hash) :
    hash_(std::move(hash)),
    leechers_(),
    seeders_(),
    downloads_(0) {}

Server::Server(boost::asio::io_context &io_context, int16_t port) :
    socket_(io_context, udp::endpoint(udp::v4(), port)) {}

void Server::start() {
    for (;;) {
        char data[max_length];
        udp::endpoint sender_endpoint;
        size_t length = socket_.receive_from(boost::asio::buffer(data, max_length), sender_endpoint);

        socket_.send_to(boost::asio::buffer(data, length), sender_endpoint);
    }
}
