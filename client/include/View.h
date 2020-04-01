#ifndef HSE_2020_TORRENT_PROJECT_VIEW_H
#define HSE_2020_TORRENT_PROJECT_VIEW_H

#include <unordered_map>
#include "libtorrent/fwd.hpp"
#include "libtorrent/torrent_status.hpp"

class View {


public:
    // ключ - торрент, значение - хэш
    void updateTorrents(std::vector<lt::torrent_status> st);

    lt::torrent_status &getTorrent(const std::string &);

    void updateCurrentTorrents();

    std::unordered_map<std::string, lt::sha1_hash> torrent_to_hash;

    std::vector<lt::torrent_status const *> current_handles;
    std::unordered_map<lt::torrent_handle, lt::torrent_status> session_handles;
    std::unordered_map<std::string, lt::torrent_handle> converter;
private:
};

#endif //HSE_2020_TORRENT_PROJECT_VIEW_H
