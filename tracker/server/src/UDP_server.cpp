#include <iostream>
#include <boost/endian/conversion.hpp>

#include "UDP_server.hpp"

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

    std::atomic<bool> Server::in_process = true;
    Server::Server(boost::asio::io_context &io_context,
                   DataBase::TorrentDataBase &db,
                   uint16_t port,
                   int32_t request_interval,
                   bool silent_mode) :
            socket_(io_context, udp::endpoint(udp::v4(), port)),
            request_interval_(request_interval),
            silent_mode_(silent_mode),
            db_(db) {}

    void Server::start() {
        socket_.non_blocking(true);
        std::vector<uint8_t> message;
        udp::endpoint sender_endpoint;
        while (in_process) {
            message.assign(PACKET_SIZE, 0);
            boost::system::error_code error;
            std::size_t message_length = socket_.receive_from(boost::asio::buffer(message), sender_endpoint, 0, error);
            if (!error) {
                message.resize(message_length);//?
                process_request(std::move(message), std::move(sender_endpoint));
            } else if (error == boost::asio::error::would_block) {
                std::this_thread::sleep_for(chrono::microseconds(SLEEP_TIME));
            } else {
                throw std::runtime_error("Server::start() socket error\n");
            }
        }
    }

    void Server::process_request(std::vector<uint8_t> message, udp::endpoint sender_endpoint) {
        Request request = parse_UDP_request(message, sender_endpoint);

        print_request(request, sender_endpoint);

        Response response = Response();

        if (!request.error_message.empty()) {
            response = handle_error(request);
        }
        if (request.action == DataBase::ActionType::CONNECT) {
            response = handle_connect(request);
        }
        if (request.action == DataBase::ActionType::ANNOUNCE) {
            response = handle_announce(request);
        }
        if (request.action == DataBase::ActionType::SCRAPE) {
            response = handle_scrape(request);
        }

        print_response(response, sender_endpoint);

        socket_.send_to(boost::asio::buffer(make_UDP_response(response)), sender_endpoint);
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

    Request Server::parse_UDP_request(const std::vector<uint8_t> &message, const udp::endpoint &sender_endpoint) {
        Request request;
        request.sender.ip = sender_endpoint.address().to_v4().to_ulong();
        request.sender.port = sender_endpoint.port();

        const uint8_t *iter = message.data();

        if (message.size() < CONNECT_REQUEST_SIZE) {
            request.error_message = "Bad request: request_size=" + std::to_string(message.size()) + "  less than CONNECT_REQUEST_SIZE";
            return request;
        }

        load_value(request.connection_id, iter);
        load_value(request.action, iter);
        if (DataBase::ActionType::CONNECT > request.action ||
            request.action > DataBase::ActionType::ERROR) { //check enum
            request.error_message = "Bad request: bad Action field";
            return request;
        }
        load_value(request.transaction_id, iter);

        if (request.action == DataBase::ActionType::CONNECT) {
            // nothing
        }
        if (request.action == DataBase::ActionType::ANNOUNCE) {
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
            if (DataBase::EventType::NONE > request.event || request.event > DataBase::EventType::STOPPED) {
                request.error_message = "Bad request: bad Event field";
                return request;
            }
            load_value(request.ip, iter);
            load_value(request.key, iter);
            load_value(request.num_want, iter);
            load_value(request.port, iter); //use?
        }

        if (request.action == DataBase::ActionType::SCRAPE) {
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

        if (response.action == DataBase::ActionType::CONNECT) {
            store_value(response.connection_id, iter);
        }

        if (response.action == DataBase::ActionType::ANNOUNCE) {
            store_value(response.interval, iter);
            store_value(response.leechers, iter);
            store_value(response.seeders, iter);
            for (const auto &peer : response.peers) {
                store_value(peer.ip, iter);
                store_value(peer.port, iter);
            }
        }

        if (response.action == DataBase::ActionType::SCRAPE) {
            for (const auto &info : response.scrape_info) {
                store_value(info.complete, iter);
                store_value(info.downloaded, iter);
                store_value(info.incomplete, iter);
            }
        }

        if (response.action == DataBase::ActionType::ERROR) {
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
        response.leechers = db_.count_leechers(info_hash);
        response.seeders = db_.count_seeders(info_hash);
        response.peers = db_.get_peer_list(info_hash, request.num_want);

        db_.update_peer_list(info_hash, request.sender, request.event);

        return response;
    }

    Response Server::handle_scrape(const Request &request) {
        Response response;
        response.action = request.action; //  == 2
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        for (const auto &cur_info_hash : request.info_hashes) {
            response.scrape_info.emplace_back();
            response.scrape_info.back().complete = db_.count_seeders(cur_info_hash);
            response.scrape_info.back().downloaded = db_.count_downloads(cur_info_hash);
            response.scrape_info.back().incomplete = db_.count_leechers(cur_info_hash);
        }

        return response;
    }

    Response Server::handle_error(const Request &request) {
        Response response;
        response.action = DataBase::ActionType::ERROR; //  == 3
        response.transaction_id = request.transaction_id;
        response.sender = request.sender;

        response.error_message = request.error_message;

        return response;
    }


    void Server::print_request(const Request &request, const udp::endpoint &sender_endpoint) const {
        if (silent_mode_) {
            return;
        }
        std::stringstream info_message;
        info_message << "->REQUEST action=";
        switch (request.action) {
            case DataBase::ActionType::CONNECT:
                info_message << "Connect ";
                break;
            case DataBase::ActionType::ANNOUNCE:
                info_message << "Announce ";
                break;
            case DataBase::ActionType::SCRAPE:
                info_message << "Scrape ";
                break;
            case DataBase::ActionType::ERROR:
                info_message << "Error ";
                break;
        }

        if (request.action == DataBase::ActionType::ANNOUNCE) {
            info_message << "event=";
            switch (request.event) {
                case DataBase::EventType::NONE:
                    info_message << "None ";
                    break;
                case DataBase::EventType::COMPLETED:
                    info_message << "Completed ";
                    break;
                case DataBase::EventType::STARTED:
                    info_message << "Started ";
                    break;
                case DataBase::EventType::STOPPED:
                    info_message << "Stopped ";
                    break;
                default:
                    info_message << " [ERROR not DataBase::EventType] ";
            }
        }
        info_message << " from " << sender_endpoint.address().to_string()
                     << " " << sender_endpoint.port() << '\n';

        std::cout << info_message.str();
    }

    void Server::print_response(const Response &response, const udp::endpoint &sender_endpoint) const {
        if (silent_mode_) {
            return;
        }
        std::stringstream info_message;
        info_message << "RESPONSE action=";
        switch (response.action) {
            case DataBase::ActionType::CONNECT:
                info_message << "Connect ";
                break;
            case DataBase::ActionType::ANNOUNCE:
                info_message << "Announce ";
                break;
            case DataBase::ActionType::SCRAPE:
                info_message << "Scrape ";
                break;
            case DataBase::ActionType::ERROR:
                info_message << "Error ";
                break;
        }

        if (response.action == DataBase::ActionType::ERROR) {
            info_message << response.error_message << " ";
        }

        info_message << "[ " << response.peers.size() << " in the list ]";

        info_message << " to " << sender_endpoint.address().to_string()
                     << " " << sender_endpoint.port() << '\n';

        std::cout << info_message.str();
    }
} //namespace UDP_server
