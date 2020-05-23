//
// Created by kirill on 04/04/2020.
//
#include <set>
#include <iostream>
#include "TorrentDataBase.hpp"

namespace DataBase {
    bool Peer::operator<(const Peer &other) const noexcept {
        return peer_id < other.peer_id;
    }

    TorrentDataBase::TorrentDataBase() :
            random_(std::random_device()()) {}

    int32_t TorrentDataBase::count_leechers(const Hash_t &info_hash) {
        return torrents_[info_hash].leechers;
    }

    int32_t TorrentDataBase::count_seeders(const Hash_t &info_hash) {
        return torrents_[info_hash].seeders;
    }

    int32_t TorrentDataBase::count_downloads(const Hash_t &info_hash) {
        return torrents_[info_hash].downloads;
    }

    std::vector<Peer> TorrentDataBase::get_peer_list(const Hash_t &info_hash, int32_t num_want) {
        std::vector<Peer> peers;

        const auto &peer_set = torrents_[info_hash].all_peers;

        if (num_want <= 0) {
            num_want = DEFAULT_ANNOUNCE_PEERS;
        }
        num_want = std::min(num_want, MAX_ANNOUNCE_PEERS);
        num_want = std::min(num_want, static_cast<int32_t>(peer_set.size()));

        std::set<Peer> selected;
        while (static_cast<int32_t>(selected.size()) < num_want) {
            auto random_peer = *peer_set.nth(random_() % peer_set.size());
            selected.insert(random_peer);
        }
        peers.reserve(selected.size());
        for (const auto &peer : selected) {
            peers.push_back(peer);
        }
        return peers;
    }

    std::string_view TorrentDataBase::update_peer_list(const Hash_t &info_hash, const Peer &peer, EventType event) {
        auto &torrent = torrents_[info_hash];
        auto old_peer_ptr = torrent.all_peers.find(peer);

        if (event == EventType::NONE) {
            //nothing
        }
        if (event == EventType::COMPLETED) {
            if (torrent.all_peers.count(peer) != 0) {
                if (peer.left != 0) {
                    return "Not completed actually";
                }

                if (!old_peer_ptr->completed) {
                    torrent.seeders++;
                    torrent.downloads++;
                    torrent.leechers--;

                    auto new_peer = *old_peer_ptr;
                    new_peer.completed = true;

                    torrent.all_peers.erase(old_peer_ptr);
                    torrent.all_peers.insert(new_peer);
                }
            }
        }
        if (event == EventType::STARTED) {
            if (torrent.all_peers.count(peer) == 0) {
                if (peer.left != 0) {
                    torrent.leechers++;
                } else {
                    torrent.seeders++;
                }
                torrent.all_peers.insert(peer);
            }
        }
        if (event == EventType::STOPPED) {
            if (torrent.all_peers.count(peer) != 0) {
                if (old_peer_ptr->completed) {
                    torrent.seeders--;
                } else {
                    torrent.leechers--;
                }
                torrent.all_peers.erase(old_peer_ptr);
            }
        }

        return "";
    }

}
