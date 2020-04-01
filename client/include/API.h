#ifndef HSE_2020_TORRENT_PROJECT_API_H
#define HSE_2020_TORRENT_PROJECT_API_H

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/session.hpp>
#include "View.h"
#include "InfoHelper.h"
#include <iostream>
#include <unordered_map>

class API {
public:
    API();

    void createDownload(const std::string &file_name, const std::string &path); // после конца надо удалить из вектора

    void pauseDownload(const std::string &file_name);

    void resumeDownload(const std::string &file_name);

    void removeDownload(const std::string &file_name);

    void takeUpdates(int query_type, const std::string &file_name = "",
                     const std::string &path = ""); // потом сделать через шаблоны

    void parseUpdates();

    void startDownload();

    void setPath(const std::string &path);

    void setFile(const std::string &file_name);

    TorrentInfo getInfo(const std::string &file_name);

    View view;
private:
    int query_type_ = 0;
    std::string file_name_;
    std::string path_;

    InfoHelper ih;
    lt::add_torrent_params p;
    lt::session ses;
};


#endif //HSE_2020_TORRENT_PROJECT_API_H
