#ifndef HSE_2020_TORRENT_PROJECT_API_H
#define HSE_2020_TORRENT_PROJECT_API_H

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include "client.h"
#include <iostream>
#include <unordered_map>

class API{
public:
    void enter_file(const std::string& file);
    void enter_dir(const std::string& path);

    void pause_download(std::string& file_name);
    void resume_download(std::string& file_name);

    void remove_download(std::string& file_name);

    void start_download();
    client cl;
    std::unordered_map <std::string, lt::sha1_hash> torrent_to_hash; // ключ - торрент, значение - хэш
};


#endif //HSE_2020_TORRENT_PROJECT_API_H
