#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <set>

using boost::asio::ip::udp;

class Peer{
public:
    Peer() = default;
    explicit Peer(udp::endpoint endpoint);
    ~Peer() = default;
    uint32_t ip() const noexcept;
    uint16_t port() const noexcept;
    udp::endpoint ep() const noexcept;
    bool operator<(const Peer &other) const noexcept; // для std::set
private:
    udp::endpoint endpoint_;
    std::array<uint8_t, 20> peer_id_{};
};

class Torrent{ //?
public:
    Torrent() = default;
    explicit Torrent(const std::array<uint8_t, 20> &hash);
    std::array<uint8_t, 20> hash_{};
    std::set<Peer> seeders_;
    std::set<Peer> leechers_;
    size_t downloads_ = 0;
};

struct Request{
    int64_t connection_id;
    int32_t action;
    int32_t transaction_id;

    //if action ==  1 or 2
    std::vector<std::array<uint8_t, 20>> info_hashes;

    //if action == 1
    std::array<uint8_t, 20> peer_id;
    int64_t downloaded;
    int64_t left;
    int64_t uploaded;
    int32_t event;
    uint32_t ip;
    uint32_t key;
    int32_t num_want;
    uint16_t port;

    Peer sender;
    bool correct = true;
};

struct Response{
    int32_t action;
    int32_t transaction_id;
    // if action == 0  <---- вот из-за этого я и хочу разделить запросы
    int64_t connection_id;
    // if action == 1
    int32_t interval;
    int32_t leechers;
    int32_t seeders;
    std::vector<Peer> peers;

    struct Scrape{
        int32_t complete;
        int32_t downloaded;
        int32_t incomplete;
    };

    //action == 2
    std::vector<Scrape> scrape_info;

    //action == 3
    std::string error;

    Peer sender;
};

class Server{
public:
    explicit Server(boost::asio::io_context &io_context, int16_t port = 8000); //сделать приватным?
    void start();
    static Request parse_UDP_request(const std::vector<uint8_t> &message, const udp::endpoint &ep);
    static std::vector<uint8_t> make_UDP_response(const Response &response); // optional??
    static Response handle_connect(const Request &request);
    Response handle_announce(const Request &request);
    Response handle_scrape(const Request &request);
    //Response handle_error(const Request &request);
private:
    udp::socket socket_;
    std::map<std::array<uint8_t, 20>, Torrent> torrents_;
    static constexpr std::size_t max_length = 1024;
};

