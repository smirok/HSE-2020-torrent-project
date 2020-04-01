#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <set>

namespace UDP_server {
    using boost::asio::ip::udp;
    constexpr std::size_t PACKET_SIZE = 1024;
    constexpr std::size_t HASH_SIZE = 20;
    constexpr std::size_t CONNECT_REQUEST_SIZE = 16;
    constexpr std::size_t ANNOUNCE_REQUEST_SIZE = 98;

    class Peer {
    public:
        Peer() = default;
        explicit Peer(udp::endpoint endpoint);
        uint32_t ip() const noexcept;
        uint16_t port() const noexcept;
        udp::endpoint ep() const noexcept;
        bool operator<(const Peer &other) const noexcept; // для std::set
    private:
        udp::endpoint endpoint_;
        std::array<uint8_t, HASH_SIZE> peer_id_{};
    };

    class Torrent {
    public:
        Torrent() = default;
        explicit Torrent(const std::array<uint8_t, HASH_SIZE> &hash);
        std::array<uint8_t, HASH_SIZE> hash_{};
        std::set<Peer> seeders_;
        std::set<Peer> leechers_;
        //может вообще хранить один set  и сортировать по кол-ву скачанный байт?
        size_t downloads_ = 0;
    };

    enum class ActionType : int32_t {
        CONNECT, ANNOUNCE, SCRAPE, ERROR
    };
    enum class EventType : int32_t {
        NONE, COMPLETED, STARTED, STOPPED
    };

    struct Request {
        int64_t connection_id;
        ActionType action;
        int32_t transaction_id;

        //if action ==  1 or 2
        std::vector<std::array<uint8_t, HASH_SIZE>> info_hashes;

        //if action == 1
        std::array<uint8_t, HASH_SIZE> peer_id;
        int64_t downloaded;
        int64_t left;
        int64_t uploaded;
        EventType event;
        uint32_t ip;
        uint32_t key;
        int32_t num_want;
        uint16_t port;

        Peer sender;
        std::string error_message;
    };

    struct Response {
        ActionType action;
        int32_t transaction_id;
        // if action == 0  <---- вот из-за этого я и хочу разделить запросы
        int64_t connection_id;
        // if action == 1
        int32_t interval;
        int32_t leechers;
        int32_t seeders;
        std::vector<Peer> peers;

        struct Scrape {
            int32_t complete;
            int32_t downloaded;
            int32_t incomplete;
        };

        //action == 2
        std::vector<Scrape> scrape_info;

        //action == 3
        std::string error_message;

        Peer sender;
    };

    class Server {
    public:
        explicit Server(boost::asio::io_context &io_context,
                        uint16_t port,
                        int32_t request_interval,
                        bool silent_mode);
        void start();
        static Request parse_UDP_request(const std::vector<uint8_t> &message, const udp::endpoint &ep);
        static std::vector<uint8_t> make_UDP_response(const Response &response);
        Response handle_connect(const Request &request);
        Response handle_announce(const Request &request);
        Response handle_scrape(const Request &request);
        Response handle_error(const Request &request);
        void print_request(const Request &request);
        void print_response(const Response &response);
    private:
        udp::socket socket_;
        int32_t request_interval_;
        bool silent_mode_;
        std::map<std::array<uint8_t, HASH_SIZE>, Torrent> torrents_;
    };
} //namespace UDP_server