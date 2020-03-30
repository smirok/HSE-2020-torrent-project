#include "server.hpp"

#include <utility>
#include <boost/endian/conversion.hpp>

/* константы, конфиг?? (как его делатб)
 *
 * Сделать отчёт об ошибках и его отправку доделать
 *
 * Сделать фабрику запросов и ответов (три вида запросов, четыре вида ответов)
 * Aсинхронность
 * HTTP
 * */

namespace UDP_server {
    using namespace boost::asio;
    using namespace boost::endian;

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
            return port() < other.port();
    }

    Torrent::Torrent(const std::array<uint8_t, HASH_SIZE> &hash) :
            hash_(hash),
            seeders_(),
            leechers_(),
            downloads_(0) {}

    Server::Server(boost::asio::io_context &io_context, int16_t port) :
            socket_(io_context, udp::endpoint(udp::v4(), port)) {}

    void Server::start() {
        for (;;) {
            std::vector<uint8_t> message(PACKET_SIZE);
            udp::endpoint sender_endpoint;
            std::cout << "receiving message\n";
            size_t message_length = socket_.receive_from(boost::asio::buffer(message), sender_endpoint);
            message.resize(message_length);//?
            std::cout << "got it\n";
            Request request = parse_UDP_request(message, sender_endpoint);
            Response response;
            if (!request.is_correct)
                request.action = ActionType ::ERROR;
            switch (request.action) {
                case ActionType::CONNECT:
                    response = handle_connect(request);
                    break;
                case ActionType::ANNOUNCE:
                    response = handle_announce(request);
                    break;
                case ActionType::SCRAPE:
                    response = handle_scrape(request);
                    break;
                case ActionType::ERROR:
                    response = handle_error(request);
                    break;
            }
            std::cout << "sending response " << static_cast<int>(response.action)
                      << " to " << response.sender.ep().address().to_string()
                      << ' ' << response.sender.port() << '\n';
            socket_.send_to(boost::asio::buffer(make_UDP_response(response), message_length), response.sender.ep());
        }
    }

    template<typename T>
    void load_value(T &value, const uint8_t *&source) { //only for primitive types (8 bytes or less)
        value = endian_load<T, sizeof(T), order::big>(source);
        source += sizeof(T);
    }

    template<>
    void load_value(std::array<uint8_t, HASH_SIZE> &value, const uint8_t *&source) {
        std::copy(source, source + sizeof(value), value.data());
        source += sizeof(value);
    }

    template<typename T>
    void store_value(T value, uint8_t *&destination) { //only for primitive types (8 bytes or less)
        endian_store<T, sizeof(T), order::big>(destination, value);
        destination += sizeof(T);
    }

    Request Server::parse_UDP_request(const std::vector<uint8_t> &message, const udp::endpoint &ep) {
        Request request;
        const uint8_t *iter = message.data();

        if (message.size() < 16) { // в константу
            request.is_correct = false;
            return request;
        }


        load_value(request.connection_id, iter);
        load_value(request.action, iter);
        if (ActionType::CONNECT > request.action || request.action > ActionType::ERROR) {
            request.is_correct = false;
            return request;
        }
        load_value(request.transaction_id, iter);

        if (request.action == ActionType::CONNECT) {
            // nothing
        }
        if (request.action == ActionType::ANNOUNCE) {
            if (message.size() < 98) {
                request.is_correct = false;
                return request;
            }

            request.info_hashes.emplace_back();
            load_value(request.info_hashes.back(), iter);

            load_value(request.peer_id, iter);
            load_value(request.downloaded, iter);
            load_value(request.left, iter);
            load_value(request.uploaded, iter);
            load_value(request.event, iter);
            if (EventType::NONE > request.event || request.event > EventType::STOPPED) {
                request.is_correct = false;
                return request;
            }
            load_value(request.ip, iter);
            load_value(request.key, iter);
            load_value(request.num_want, iter);
            load_value(request.port, iter);
        }

        if (request.action == ActionType::SCRAPE) {
            size_t bytes_left = message.size() - 16; // константаааа
            if (bytes_left == 0 || bytes_left % HASH_SIZE != 0) {
                request.is_correct = false;
                return request;
            }

            while (bytes_left > 0) {
                request.info_hashes.emplace_back();
                load_value(request.info_hashes.back(), iter);
                bytes_left -= HASH_SIZE;
            }
        }

        request.sender = Peer(ep);
        return request;
    }

    std::vector<uint8_t> Server::make_UDP_response(const Response &response) {
        std::vector<uint8_t> packet(PACKET_SIZE);
        uint8_t *iter = packet.data();

        store_value(response.action, iter);
        store_value(response.transaction_id, iter);

        if (response.action == ActionType::CONNECT) {
            store_value(response.connection_id, iter);
        }

        if (response.action == ActionType::ANNOUNCE) {
            store_value(response.interval, iter);
            store_value(response.leechers, iter);
            store_value(response.seeders, iter);
            for (const auto &peer : response.peers) {
                store_value(peer.ip(), iter);
                store_value(peer.port(), iter);
            }
        }

        if (response.action == ActionType::SCRAPE) {
            for (const auto &info : response.scrape_info) {
                store_value(info.complete, iter);
                store_value(info.downloaded, iter);
                store_value(info.incomplete, iter);
            }
        }

        if (response.action == ActionType::ERROR) {
            for (const auto &symbol : response.error_message) {
                store_value(symbol, iter);
            }
        }

        packet.resize(iter - packet.data());//?
        return packet;
    }

    Response Server::handle_connect(const Request &request) {
        Response response;
        response.action = request.action; //  == 0
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        response.connection_id = request.connection_id;

        return response;
    }

    Response Server::handle_announce(const Request &request) {
        Response response;
        response.action = request.action; // == 1
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        response.interval = 120; // естественно в константу

        if (torrents_.find(request.info_hashes[0]) == torrents_.end()) {
            torrents_[request.info_hashes[0]] = Torrent(request.info_hashes[0]);
        }

        auto &seeders_set = torrents_[request.info_hashes[0]].seeders_;
        auto &leechers_set = torrents_[request.info_hashes[0]].leechers_;

        response.leechers = leechers_set.size();
        response.seeders = seeders_set.size();
        //incorrect, should use num_want ->
        for (const auto &peer : seeders_set) {
            response.peers.push_back(peer);
        }
        //incorrect, should use num_want ->
        for (const auto &peer : leechers_set) {
            response.peers.push_back(peer);
        }

        if (request.event == EventType::NONE) {
            //nothing
        }
        if (request.event == EventType::COMPLETED) {
            leechers_set.erase(request.sender);
            seeders_set.insert(request.sender);
        }
        if (request.event == EventType::STARTED) {
            leechers_set.insert(request.sender);
        }
        if (request.event == EventType::STOPPED) {
            leechers_set.erase(request.sender);
            seeders_set.erase(request.sender);
        }

        return response;
    }

    Response Server::handle_scrape(const Request &request) {
        Response response;
        response.action = request.action; //  == 2
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        for (const auto &cur_info_hash : request.info_hashes) {
            if (torrents_.find(cur_info_hash) == torrents_.end()) {
                torrents_[cur_info_hash] = Torrent(cur_info_hash);
            }

            response.scrape_info.emplace_back();
            response.scrape_info.back().complete = torrents_[cur_info_hash].seeders_.size();
            response.scrape_info.back().downloaded = torrents_[cur_info_hash].downloads_;
            response.scrape_info.back().incomplete = torrents_[cur_info_hash].leechers_.size();
        }

        return response;
    }

    Response Server::handle_error(const Request &request) {
        Response response;
        response.action = ActionType::ERROR; //  == 2
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        response.error_message = "Bad request";

        return response;
    }
} //namespace UDP_server
