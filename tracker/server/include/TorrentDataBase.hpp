#pragma once

#include <random>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

namespace DataBase {

    constexpr std::size_t HASH_SIZE = 20;
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

    enum class ActionType : int32_t {
        CONNECT, ANNOUNCE, SCRAPE, ERROR
    };

    enum class EventType : int32_t {
        NONE, COMPLETED, STARTED, STOPPED
    };

    class Peer {
    public:
        bool operator<(const Peer &other) const noexcept;

    public:
        uint32_t ip{};
        uint16_t port{};
        Hash_t peer_id{};
        int64_t downloaded = 0;
        int64_t left = 0;
        int64_t uploaded = 0;
        bool completed = false;
    };

    class Torrent {
    public:
        ordered_set<Peer> all_peers{};
        std::size_t leechers = 0;
        std::size_t seeders = 0;
        std::size_t downloads = 0;
    };

    class TorrentDataBase {
    public:
        TorrentDataBase();
        int32_t count_leechers(const Hash_t &info_hash);
        int32_t count_seeders(const Hash_t &info_hash);
        int32_t count_downloads(const Hash_t &info_hash);
        std::vector<Peer> get_peer_list(const Hash_t &info_hash, int32_t num_want);
        void update_peer_list(const Hash_t &info_hash, const Peer &peer, EventType event);

    private:
        std::mt19937 random_;
        std::map<Hash_t, Torrent> torrents_;
    };
} //namespace DataBase