#include "server.hpp"

#include <utility>
#include <boost/endian/conversion.hpp>

/* enum-ы, константы, конфиг?? (как его делатб)
 *
 * Сделать отчёт об ошибках и его отправку
 *
 * Сделать фабрику запросов и ответов (три вида запросов, четыре вида ответов)
 * Aсинхронность
 * HTTP
 * */


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

Torrent::Torrent(const std::array<uint8_t, 20> &hash) :
        hash_(hash),
        seeders_(),
        leechers_(),
        downloads_(0) {}

Server::Server(boost::asio::io_context &io_context, int16_t port) :
        socket_(io_context, udp::endpoint(udp::v4(), port)) {}

void Server::start() {
    for (;;) {
        std::vector<uint8_t> message(max_length);
        udp::endpoint sender_endpoint;
        std::cout << "receiving message\n";
        size_t message_length = socket_.receive_from(boost::asio::buffer(message), sender_endpoint);
        message.resize(message_length);
        std::cout << "got it\n";
        Request request = parse_UDP_request(message, sender_endpoint);
        Response response = Response();
        if (!request.correct)
            continue; // should send error response
        switch (request.action) {
            case 0:
                response = handle_connect(request);
                break;
            case 1:
                response = handle_announce(request);
                break;
            case 2:
                response = handle_scrape(request);
                break;
            default:
                break;
        }
        std::cout << "sending response "  << response.action << " to " << response.sender.ep().address().to_string() << ' ' << response.sender.port() << '\n';
        socket_.send_to(boost::asio::buffer(make_UDP_response(response), message_length), response.sender.ep());
    }
}

template<typename T>
void load_value(T &value, const uint8_t *&source) { //only for primitive types (8 bytes or less)
    value = endian_load<T, sizeof(T), order::big>(source);
    source += sizeof(T);
}

template<>
void load_value(std::array<uint8_t, 20> &value, const uint8_t *&source) {
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
        request.correct = false;
        return request;
    }


    load_value(request.connection_id, iter);
    load_value(request.action, iter);
    load_value(request.transaction_id, iter);

    if (request.action == 0) { // enum!!!!
        // nothing
    }
    if (request.action == 1) {
        if (message.size() < 98) {
            request.correct = false;
            return request;
        }

        request.info_hashes.emplace_back();
        load_value(request.info_hashes.back(), iter);

        load_value(request.peer_id, iter);
        load_value(request.downloaded, iter);
        load_value(request.left, iter);
        load_value(request.uploaded, iter);
        load_value(request.event, iter);
        load_value(request.ip, iter);
        load_value(request.key, iter);
        load_value(request.num_want, iter);
        load_value(request.port, iter);
    }

    if (request.action == 2) {
        size_t bytes_left = message.size() - 16; // константаааа
        if (bytes_left == 0 || bytes_left % 20 != 0) {
            request.correct = false;
            return request;
        }

        while (bytes_left > 0) {
            request.info_hashes.emplace_back();
            load_value(request.info_hashes.back(), iter);
            bytes_left -= 20;
        }
    }

    request.sender = Peer(ep);
    return request;
}

std::vector<uint8_t> Server::make_UDP_response(const Response &response) {
    std::vector<uint8_t> packet(max_length);
    uint8_t *iter = packet.data();

    store_value(response.action, iter);
    store_value(response.transaction_id, iter);

    if (response.action == 0) {
        store_value(response.connection_id, iter);
    }

    if (response.action == 1) {
        store_value(response.interval, iter);
        store_value(response.leechers, iter);
        store_value(response.seeders, iter);
        for (auto &peer : response.peers) {
            store_value(peer.ip(), iter);
            store_value(peer.port(), iter);
        }
    }

    if (response.action == 2) {
        for (const auto &info : response.scrape_info) {
            store_value(info.complete, iter);
            store_value(info.downloaded, iter);
            store_value(info.incomplete, iter);
        }
    }

    if (response.action == 3) {
        /*TODO error*/
    }

    packet.resize(iter - packet.data());
    return packet;
}

Response Server::handle_connect(const Request &request) {
    Response response;

    response.action = request.action; //  == 0
    response.transaction_id = request.transaction_id;
    response.connection_id = request.connection_id;
    response.sender = request.sender;

    return response;
}

Response Server::handle_announce(const Request &request) {
    Response response;

    response.action = 1; // enum
    response.transaction_id = request.transaction_id;
    response.interval = 120; // естественно в константу
    response.sender = request.sender;

    if (torrents_.find(request.info_hashes[0]) == torrents_.end()) {
        torrents_[request.info_hashes[0]] = Torrent(request.info_hashes[0]);
    }

    auto &seeders_set  = torrents_[request.info_hashes[0]].seeders_;
    auto &leechers_set = torrents_[request.info_hashes[0]].leechers_;

    response.leechers = leechers_set.size();
    response.seeders  = seeders_set.size();
    //incorrect, should use num_want ->
    for (const auto& peer : seeders_set) {
        response.peers.push_back(peer);
    }
    //incorrect, should use num_want ->
    for (const auto& peer : leechers_set) {
        response.peers.push_back(peer);
    }

    if (request.event == 0) { //enum-чики
        //nothing
    }
    if (request.event == 1) {
        leechers_set.erase(request.sender);
        seeders_set.insert(request.sender);
    }
    if (request.event == 2) {
        leechers_set.insert(request.sender);
    }
    if (request.event == 3) {
        leechers_set.erase(request.sender);
        seeders_set.erase(request.sender);
    }

    return response;
}

Response Server::handle_scrape(const Request &request) {
    Response response;

    response.action = 2; // enum
    response.transaction_id = request.transaction_id;
    response.sender = request.sender;

    for (const auto &cur_info_hash : request.info_hashes) {
        if (torrents_.find(cur_info_hash) == torrents_.end()) {
            torrents_[cur_info_hash] = Torrent(cur_info_hash);
        }

        response.scrape_info.emplace_back();
        response.scrape_info.back().complete   = torrents_[cur_info_hash].seeders_.size();
        response.scrape_info.back().downloaded = torrents_[cur_info_hash].downloads_;
        response.scrape_info.back().incomplete = torrents_[cur_info_hash].leechers_.size();
    }

    return response;
}

