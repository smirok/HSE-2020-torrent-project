#pragma once

#include <random>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <map>

namespace DataBase {
    constexpr std::size_t HASH_SIZE = 20;
    constexpr int32_t DEFAULT_ANNOUNCE_PEERS = 50;
    constexpr int32_t MAX_ANNOUNCE_PEERS = 150;
    using Hash_t = std::array<uint8_t, HASH_SIZE>;

    using boost::multi_index::multi_index_container;
    using boost::multi_index::indexed_by;
    using boost::multi_index::ranked_non_unique;
    using boost::multi_index::identity;

    template<typename T>
    using ranked_set =  multi_index_container<T, indexed_by<ranked_non_unique<identity<T>>>>;

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
        ranked_set<Peer> all_peers;
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
        std::string_view update_peer_list(const Hash_t &info_hash, const Peer &peer, EventType event);

    private:
        std::mt19937 random_;
        std::map<Hash_t, Torrent> torrents_;
    };
} //namespace DataBase