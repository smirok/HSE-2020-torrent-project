#include <iostream>
#include <boost/endian/conversion.hpp>
#include <random>
#include <set>

#include "server.hpp"

/* константы, конфиг?? (как его делатб)
 *
 * Сделать отчёт об ошибках и его отправку доделать
 *
 * Разделить на работу с запросами и базу дынных
 * Aсинхронность
 * HTTP
 * */

namespace UDP_server {
    using namespace boost::asio;
    using namespace boost::endian;

    uint32_t Peer::ip() const noexcept {
        return ep.address().to_v4().to_ulong();
    }

    uint16_t Peer::port() const noexcept {
        return ep.port();
    }

    bool Peer::operator<(const Peer &other) const noexcept {
        return peer_id < other.peer_id;
    }

    Server::Server(boost::asio::io_context &io_context,
                   uint16_t port,
                   int32_t request_interval,
                   bool silent_mode) :
            socket_(io_context, udp::endpoint(udp::v4(), port)),
            request_interval_(request_interval),
            silent_mode_(silent_mode),
            random_(std::random_device()()) {}

    void Server::start() {
        for (;;) {
            std::vector<uint8_t> message(PACKET_SIZE);
            udp::endpoint sender_endpoint;
            std::size_t message_length = socket_.receive_from(boost::asio::buffer(message), sender_endpoint);
            message.resize(message_length);//?
            Request request = parse_UDP_request(message, sender_endpoint);
            print_request(request);

            Response response = Response();

            if (!request.error_message.empty()) {
                response = handle_error(request);
            }
            if (request.action == ActionType::CONNECT) {
                response = handle_connect(request);
            }
            if (request.action == ActionType::ANNOUNCE) {
                response = handle_announce(request);
            }
            if (request.action == ActionType::SCRAPE) {
                response = handle_scrape(request);
            }

            print_response(response);

            socket_.send_to(boost::asio::buffer(make_UDP_response(response), message_length), response.sender.ep);
        }
    }

    template<typename T>
    void load_value(T &value, const uint8_t *&source) { //only for primitive types (8 bytes or less)
        value = endian_load<T, sizeof(T), order::big>(source);
        source += sizeof(T);
    }

    template<>
    void load_value(Hash_t &value, const uint8_t *&source) {
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
        request.sender.ep = ep;
        const uint8_t *iter = message.data();

        if (message.size() < CONNECT_REQUEST_SIZE) {
            request.error_message = "Bad request: request size less than CONNECT_REQUEST_SIZE";
            return request;
        }


        load_value(request.connection_id, iter);
        load_value(request.action, iter);
        if (ActionType::CONNECT > request.action || request.action > ActionType::ERROR) { //check enum
            request.error_message = "Bad request: bad Action field";
            return request;
        }
        load_value(request.transaction_id, iter);

        if (request.action == ActionType::CONNECT) {
            // nothing
        }
        if (request.action == ActionType::ANNOUNCE) {
            if (message.size() < ANNOUNCE_REQUEST_SIZE) {
                request.error_message = "Bad request: request size less than ANNOUNCE_REQUEST_SIZE";
                return request;
            }

            request.info_hashes.emplace_back();
            load_value(request.info_hashes.back(), iter);
            load_value(request.sender.peer_id, iter);
            load_value(request.sender.downloaded, iter);
            load_value(request.sender.left, iter);
            load_value(request.sender.uploaded, iter);

            load_value(request.event, iter);
            if (EventType::NONE > request.event || request.event > EventType::STOPPED) {
                request.error_message = "Bad request: bad Event field";
                return request;
            }
            load_value(request.ip, iter);
            load_value(request.key, iter);
            load_value(request.num_want, iter);
            load_value(request.port, iter);
        }

        if (request.action == ActionType::SCRAPE) {
            std::size_t bytes_left = message.size() - CONNECT_REQUEST_SIZE;
            if (bytes_left == 0 || bytes_left % HASH_SIZE != 0) {
                request.error_message = "Bad request: incomplete hash for Scrape";
                return request;
            }

            while (bytes_left > 0) {
                request.info_hashes.emplace_back();
                load_value(request.info_hashes.back(), iter);
                bytes_left -= HASH_SIZE;
            }
        }

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

        response.interval = request_interval_;

        const Hash_t &info_hash = request.info_hashes[0];
        response.leechers = count_leechers(info_hash);
        response.seeders = count_seeders(info_hash);
        response.peers = get_peer_list(info_hash, request.num_want);

        update_peer_list(info_hash, request.sender, request.event);

        return response;
    }

    Response Server::handle_scrape(const Request &request) {
        Response response;
        response.action = request.action; //  == 2
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        for (const auto &cur_info_hash : request.info_hashes) {
            response.scrape_info.emplace_back();
            response.scrape_info.back().complete = count_seeders(cur_info_hash);
            response.scrape_info.back().downloaded = count_downloads(cur_info_hash);
            response.scrape_info.back().incomplete = count_leechers(cur_info_hash);
        }

        return response;
    }

    Response Server::handle_error(const Request &request) {
        Response response;
        response.action = ActionType::ERROR; //  == 2
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        response.error_message = request.error_message;

        return response;
    }

    int32_t Server::count_leechers(const Hash_t &info_hash) {
        return torrents_[info_hash].leechers;
    }

    int32_t Server::count_seeders(const Hash_t &info_hash) {
        return torrents_[info_hash].seeders;
    }

    int32_t Server::count_downloads(const Hash_t &info_hash) {
        return torrents_[info_hash].downloads;
    }

    std::vector<Peer> Server::get_peer_list(const Hash_t &info_hash, int32_t num_want) {
        std::vector<Peer> peers;

        const auto &peer_set = torrents_[info_hash].all_peers;

        if (num_want <= 0) {
            num_want = DEFAULT_ANNOUNCE_PEERS;
        }
        num_want = std::min(num_want, MAX_ANNOUNCE_PEERS);
        num_want = std::min(num_want, (int32_t) peer_set.size());

        std::set<Peer> selected;
        while ((int32_t) selected.size() < num_want) {
            selected.insert(*peer_set.find_by_order(random_() % peer_set.size()));
        }
        for (auto &peer : selected) {
            peers.push_back(peer);
        }
        return peers;
    }

    void Server::update_peer_list(const Hash_t &info_hash, const Peer &peer, EventType event) {
        auto &torrent = torrents_[info_hash];
        auto old_peer_ptr = torrent.all_peers.find(peer);

        if (old_peer_ptr == torrent.all_peers.end() && event != EventType::STARTED) {
            //bad
        }
        if (event == EventType::NONE) {
            //nothing
        }
        if (event == EventType::COMPLETED) {
            if (old_peer_ptr->completed) {
                //bad
            }
            torrent.seeders++;
            torrent.downloads++;
            torrent.leechers--;

            auto new_peer = *old_peer_ptr;
            new_peer.completed = true; // check peer.left

            torrent.all_peers.erase(old_peer_ptr);
            torrent.all_peers.insert(new_peer);
        }
        if (event == EventType::STARTED) {
            torrent.leechers++;
            torrent.all_peers.insert(peer);
        }
        if (event == EventType::STOPPED) {
            if (old_peer_ptr->completed) {
                torrent.seeders--;
            } else {
                torrent.leechers--;
            }
            torrent.all_peers.erase(old_peer_ptr);
        }
    }

    void Server::print_request(const Request &request) const {
        if (silent_mode_) {
            return;
        }
        std::stringstream info_message;

        info_message << "REQUEST action=";
        switch (request.action) {
            case ActionType::CONNECT:
                info_message << "Connect ";
                break;
            case ActionType::ANNOUNCE:
                info_message << "Announce ";
                break;
            case ActionType::SCRAPE:
                info_message << "Scrape ";
                break;
            case ActionType::ERROR:
                info_message << "Error ";
                break;
        }

        if (request.action == ActionType::ANNOUNCE) {
            info_message << "event=";
            switch (request.event) {
                case EventType::NONE:
                    info_message << "None ";
                    break;
                case EventType::COMPLETED:
                    info_message << "Completed ";
                    break;
                case EventType::STARTED:
                    info_message << "Started ";
                    break;
                case EventType::STOPPED:
                    info_message << "Stopped ";
                    break;
                default:
                    info_message << " [ERROR not EventType] ";
            }
        }
        info_message << " from " << request.sender.ep.address().to_string()
                     << " " << request.sender.port() << '\n';

        std::cout << info_message.str();
    }

    void Server::print_response(const Response &response) const {
        if (silent_mode_) {
            return;
        }
        std::stringstream info_message;
        info_message << "RESPONSE action=";
        switch (response.action) {
            case ActionType::CONNECT:
                info_message << "Connect ";
                break;
            case ActionType::ANNOUNCE:
                info_message << "Announce ";
                break;
            case ActionType::SCRAPE:
                info_message << "Scrape ";
                break;
            case ActionType::ERROR:
                info_message << "Error ";
                break;
        }

        info_message << " to " << response.sender.ep.address().to_string()
                     << " " << response.sender.port() << '\n';

        std::cout << info_message.str();
    }
} //namespace UDP_server
