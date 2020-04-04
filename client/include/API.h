#ifndef HSE_2020_TORRENT_PROJECT_API_H
#define HSE_2020_TORRENT_PROJECT_API_H

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/session.hpp>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/create_torrent.hpp"

#include "View.h"
#include "InfoHelper.h"
#include <iostream>
#include <unordered_map>
#include <fstream>

class API {
public:
    API();

    void createDownload(const std::string &file_name, const std::string &path); // после конца надо удалить из вектора

    void pauseDownload(const std::string &file_name);

    void resumeDownload(const std::string &file_name);

    void removeDownload(const std::string &file_name, bool should_delete);

    void makeTorrent(const std::string &path_to_files, const std::vector<std::string> &trackers);

    TorrentInfo getInfo(const std::string &file_name);

private:
    void setPath(const std::string &path);

    void setFile(const std::string &file_name);

    void takeUpdates();

    View view;
    InfoHelper ih;
    lt::add_torrent_params p;
    lt::session ses;
};


#endif //HSE_2020_TORRENT_PROJECT_API_H
