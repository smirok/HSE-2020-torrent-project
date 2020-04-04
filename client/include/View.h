#ifndef HSE_2020_TORRENT_PROJECT_VIEW_H
#define HSE_2020_TORRENT_PROJECT_VIEW_H

#include <unordered_map>
#include "libtorrent/fwd.hpp"
#include "libtorrent/torrent_status.hpp"

class View {
public:
    void updateTorrents(const std::vector<lt::torrent_status>& st);

    void updateCurrentTorrents();

    std::unordered_map<lt::torrent_handle, lt::torrent_status> session_handles;
    std::vector<std::unique_ptr<lt::torrent_status const>> current_handles;
    std::unordered_map<std::string, lt::torrent_handle> converter;
};

#endif //HSE_2020_TORRENT_PROJECT_VIEW_H
