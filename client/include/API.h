#ifndef HSE_2020_TORRENT_PROJECT_API_H
#define HSE_2020_TORRENT_PROJECT_API_H

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include "client.h"
#include <iostream>

class API{
public:
    void enter_file(const std::string& file);
    void enter_dir(const std::string& path);

    void start_download();
    client cl;
};


#endif //HSE_2020_TORRENT_PROJECT_API_H
