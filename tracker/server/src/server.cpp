#include "server.hpp"

#include <utility>
#include <boost/endian/conversion.hpp>

/* enum-ы, константы, конфиг?? (как его делатб)
 *
 * Сделать handle_scrape
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

Torrent::Torrent(std::string hash) :
        hash_(std::move(hash)),
        seeders_(),
        leechers_(),
        downloads_(0) {}

Server::Server(boost::asio::io_context &io_context, int16_t port) :
        socket_(io_context, udp::endpoint(udp::v4(), port)) {}

void Server::start() {
    for (;;) {
        std::vector<unsigned char> message(max_length);
        udp::endpoint sender_endpoint;
        std::cout << "receiving message\n";
        size_t length = socket_.receive_from(boost::asio::buffer(message), sender_endpoint);
        std::cout << "got it\n";
        message.resize(length); //? чёт тупо
        Request request = parse_UDP_request(message, sender_endpoint);
        Response response = Response();
        if (!request.correct)
            continue; // should send error response
        if (request.action == 0) { // enum
            response = handle_connect(request);
        } else
        if (request.action == 1) {
            response = handle_announce(request);
        } else {
            /*TODO*/
            continue;
        }
        std::cout << "sending response to " << response.sender.ep().address().to_string() << ' ' << response.sender.port() << '\n';
        socket_.send_to(boost::asio::buffer(make_UDP_response(response)), response.sender.ep());
    }
}

template<typename T>
void load_value(T &value, const uint8_t *&source) { //only for primitive types (8 bytes or less)
    value = endian_load<T, sizeof(T), order::big>(source);
    source += sizeof(T);
}

template<typename T>
void store_value(T value, uint8_t *&destination) { //only for primitive types (8 bytes or less)
    endian_store<T, sizeof(T), order::big>(destination, value);
    destination += sizeof(T);
}

Request Server::parse_UDP_request(const std::vector<unsigned char> &message, const udp::endpoint &ep) {
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

        request.info_hashes.emplace_back(reinterpret_cast<const char *>(iter), 20);
        iter += 20; // в константу

        request.peer_id = std::string(reinterpret_cast<const char *>(iter), 20);
        iter += 20;

        load_value(request.downloaded, iter);
        load_value(request.left, iter);
        load_value(request.uploaded, iter);
        load_value(request.event, iter);
        load_value(request.ip, iter);
        load_value(request.key, iter);
        load_value(request.num_want, iter);
        load_value(request.port, iter);
    }

    if (request.action == 3) {
        size_t bytes_left = message.size() - 16; // константаааа
        if (bytes_left == 0 || bytes_left % 20 != 0) {
            request.correct = false;
            return request;
        }

        while (bytes_left > 0) {
            request.info_hashes.emplace_back(reinterpret_cast<const char *>(iter), 20);
            iter += 20;
            bytes_left -= 20;
        }
    }

    request.sender = Peer(ep);
    return request;
}

std::vector<unsigned char> Server::make_UDP_response(const Response &response) {
    std::vector<unsigned char> packet(1024);
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
        /*TODO scrape*/
    }

    if (response.action == 3) {
        /*TODO error*/
    }

    packet.resize(iter - packet.data());
    return packet;
}

Response Server::handle_connect(const Request &request) {
    Response response;

    //тупо
    response.action = request.action; //  == 0
    response.transaction_id = request.transaction_id;
    response.connection_id = request.connection_id;
    response.sender = request.sender;

    return response;
}

Response Server::handle_announce(const Request &request) {
    Response response;

    response.action = 1; // const
    response.transaction_id = request.transaction_id;
    response.interval = 200; // естественно в константу

    if (torrents_.find(request.info_hashes[0]) == torrents_.end())
        torrents_[request.info_hashes[0]] = Torrent(request.info_hashes[0]);

    auto &seeders_set  = torrents_[request.info_hashes[0]].seeders_;
    auto &leechers_set = torrents_[request.info_hashes[0]].leechers_;

    response.leechers = leechers_set.size();
    response.seeders  = seeders_set.size();
    //incorrect, should use num_want ->
    for (const auto& peer : seeders_set)
        response.peers.push_back(peer);
    //incorrect, should use num_want ->
    for (const auto& peer : leechers_set)
        response.peers.push_back(peer);

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
    response.sender = request.sender;

    return response;
}
