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

#include "Linker.h"
#include "InfoHelper.h"
#include <iostream>
#include <unordered_map>
#include <fstream>

class FilesPicker {
public:
    void setMark(int32_t index, bool mark);
    std::vector<bool> getMarks();
    std::vector<FileNode> download_holder;

    static constexpr int32_t MAX_FILES = 100;
    static constexpr int32_t MAX_LENGTH = 100000;
    static constexpr int32_t ROOT = 0;
};

class API {
public:
    API();

    void prepareDownload(const std::string &file_name, const std::string &path);

    void pickDownloadFiles();

    void createDownload(const std::string &file_name);

    void pauseDownload(const std::string &file_name);

    void resumeDownload(const std::string &file_name);

    void removeDownload(const std::string &file_name, bool should_delete);

    void makeTorrent(const std::string &path_to_files,
                     const std::vector<std::string> &trackers,
                     const std::string &path_to_torrent_file,
                     const std::string &torrent_file_name);

    TorrentInfo getInfo(const std::string &file_name);

    FilesPicker picker;

private:
    void setPath(const std::string &path);

    void setFile(const std::string &file_name);

    void takeUpdates();

    Linker linker;
    InfoHelper info_helper;
    lt::add_torrent_params params;
    lt::session ses;
};


#endif //HSE_2020_TORRENT_PROJECT_API_H
