#pragma once

//#include <cstdlib>
#include <boost/asio.hpp>
#include <set>

#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

namespace UDP_server {
    using boost::asio::ip::udp;
    constexpr std::size_t PACKET_SIZE = 1024;
    constexpr std::size_t HASH_SIZE = 20;
    constexpr std::size_t CONNECT_REQUEST_SIZE = 16;
    constexpr std::size_t ANNOUNCE_REQUEST_SIZE = 98;
    constexpr int32_t DEFAULT_ANNOUNCE_PEERS = 50;
    constexpr int32_t MAX_ANNOUNCE_PEERS = 150;
    using Hash_t = std::array<uint8_t, HASH_SIZE>;

    /* структура из STL. как std::set, только с random_access за O(log n)
     * https://codeforces.com/blog/entry/11080
     */
    template<typename T>
    using ordered_set = __gnu_pbds::tree<T,
                                         __gnu_pbds::null_type,
                                         std::less<>,
                                         __gnu_pbds::rb_tree_tag,
                                         __gnu_pbds::tree_order_statistics_node_update>;

    class Peer {
    public:
        //Peer() = default;
        //explicit Peer(const Hash_t &peer_id, udp::endpoint endpoint);
        uint32_t ip() const noexcept;
        uint16_t port() const noexcept;
        bool operator<(const Peer &other) const noexcept;

    public:
        udp::endpoint ep;
        Hash_t peer_id{};
        int64_t downloaded;
        int64_t left;
        int64_t uploaded;
        bool completed = false;
    };

    class Torrent {
    public:
        Torrent() = default;
        explicit Torrent(const Hash_t &hash);
        Hash_t info_hash{}; //unused
        ordered_set<Peer> all_peers{};
        std::size_t leechers = 0;
        std::size_t seeders = 0;
        std::size_t downloads = 0;
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
        std::vector<Hash_t> info_hashes;

        //if action == 1
        Peer sender;
        EventType event;
        uint32_t ip;
        uint32_t key;
        int32_t num_want;

        uint16_t port;
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
        int32_t count_leechers(const Hash_t &info_hash);
        int32_t count_seeders(const Hash_t &info_hash);
        int32_t count_downloads(const Hash_t &info_hash);
        std::vector<Peer> get_peer_list(const Hash_t &info_hash, int32_t num_want);
        void update_peer_list(const Hash_t &info_hash, const Peer &peer, EventType event);
        void print_request(const Request &request);
        void print_response(const Response &response);
    private:
        udp::socket socket_;
        int32_t request_interval_;
        bool silent_mode_;
        std::map<Hash_t, Torrent> torrents_;
    };
} //namespace UDP_server
