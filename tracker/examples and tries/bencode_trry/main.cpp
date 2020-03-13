
#include <fstream>
#include "bencode.hpp"
#include <boost/asio.hpp>

int main() {
    std::ifstream in("example.torrent");
//    char t;
//    std:: >> t;
//    std::istreambuf_iterator<char> begin(in);
//    std::cout << *begin;
    auto data = bencode::decode(in);
    auto file = boost::get<bencode::dict>(data);
    std::cout << bencode::encode(data);
    for (auto [key, value] : file) {
        std::cout << key << '\n';
    }
    boost::asio::ip::udp::ac
    return 0;
}
