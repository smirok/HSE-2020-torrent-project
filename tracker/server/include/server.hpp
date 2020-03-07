#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <set>

using boost::asio::ip::udp; // это плохо, очень много подрубается

//to_big_endian() {
//
//}

class Peer{
    explicit Peer(udp::endpoint endpoint);
    uint32_t ip() const noexcept;
    uint16_t port() const noexcept;
    udp::endpoint ep() const noexcept;
    bool operator<(const Peer &other) const noexcept;
private:
    udp::endpoint endpoint_;
};

class Torrent{ //?
public:
    explicit Torrent(std::string hash);
    std::string hash_;
    std::set<Peer> seeders_;
    std::set<Peer> leechers_;
    size_t downloads_;
};

class Server{
public:
    explicit Server(boost::asio::io_context &io_context, int16_t port = 8000);
    void start();
private:
    udp::socket socket_;
    std::map<std::string, Torrent> torrents_;
    enum {max_length = 1024}; // 1024??
};
