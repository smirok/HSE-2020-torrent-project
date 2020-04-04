#pragma once

#include <boost/asio.hpp>
#include "TorrentDataBase.hpp"

namespace UDP_server {
    using boost::asio::ip::udp;
    constexpr std::size_t PACKET_SIZE = 1024;
    constexpr std::size_t CONNECT_REQUEST_SIZE = 16;
    constexpr std::size_t ANNOUNCE_REQUEST_SIZE = 98;
    using DataBase::Hash_t;
    using DataBase::HASH_SIZE;

    class Request {
    public:
        int64_t connection_id;
        DataBase::ActionType action;
        int32_t transaction_id;

        //if action ==  1 or 2
        std::vector<Hash_t> info_hashes;

        //if action == 1
        DataBase::Peer sender;
        DataBase::EventType event;
        uint32_t ip;
        uint32_t key;
        int32_t num_want;

        uint16_t port;
        std::string error_message;
    };

    class Response {
    public:
        DataBase::ActionType action;
        int32_t transaction_id;
        // if action == 0  <---- вот из-за этого я и хочу разделить запросы
        int64_t connection_id;
        // if action == 1
        int32_t interval;
        int32_t leechers;
        int32_t seeders;
        std::vector<DataBase::Peer> peers;

        struct Scrape {
            int32_t complete;
            int32_t downloaded;
            int32_t incomplete;
        };

        //action == 2
        std::vector<Scrape> scrape_info;

        //action == 3
        std::string error_message;

        DataBase::Peer sender;
    };

    class Server {
    public:
        explicit Server(boost::asio::io_context &io_context,
                        DataBase::TorrentDataBase &db,
                        uint16_t port,
                        int32_t request_interval,
                        bool silent_mode);
        void start();
        void process_request(std::vector<uint8_t> message, udp::endpoint sender_endpoint);
        static Request parse_UDP_request(const std::vector<uint8_t> &message, const udp::endpoint &ep);
        static std::vector<uint8_t> make_UDP_response(const Response &response);
        Response handle_connect(const Request &request);
        Response handle_announce(const Request &request);
        Response handle_scrape(const Request &request);
        Response handle_error(const Request &request);
        void print_request(const Request &request, const udp::endpoint &sender_endpoint) const;
        void print_response(const Response &response, const udp::endpoint &sender_endpoint) const;
    private:
        udp::socket socket_;
        int32_t request_interval_;
        bool silent_mode_;
        DataBase::TorrentDataBase &db_;
    };
} //namespace UDP_server
