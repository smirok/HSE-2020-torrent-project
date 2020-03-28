#ifndef HSE_2020_TORRENT_PROJECT_VIEW_H
#define HSE_2020_TORRENT_PROJECT_VIEW_H

#include <unordered_map>
#include "libtorrent/fwd.hpp"
#include "libtorrent/torrent_status.hpp"

class View{


    // ключ - торрент, значение - хэш
public:
    std::unordered_map<std::string, lt::sha1_hash> torrent_to_hash;
private:
    std::unordered_map<lt::torrent_handle,lt::torrent_status> session_handles;
    std::vector<lt::torrent_handle> current_handles;
    std::unordered_map<std::string,lt::torrent_handle> converter;
};

#endif //HSE_2020_TORRENT_PROJECT_VIEW_H
