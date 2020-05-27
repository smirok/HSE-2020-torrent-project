#ifndef HSE_2020_TORRENT_PROJECT_TORRENTAPI_H
#define HSE_2020_TORRENT_PROJECT_TORRENTAPI_H

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/session.hpp>

#include "Linker.h"
#include "InfoHelper.h"
#include <unordered_map>

struct FilesPicker {

    void setMark(int32_t index, bool mark);

    std::vector<bool> getMarks();

    std::vector<FileNode> download_holder;

    static constexpr int32_t MAX_FILES = 100;
    static constexpr int32_t MAX_LENGTH = 100000;
    static constexpr int32_t ROOT = 0;
};

class TorrentAPI {
public:
    TorrentAPI();

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

    Linker linker_;
    InfoHelper info_helper_;
    lt::add_torrent_params params_;
    lt::session session_;
};

#endif //HSE_2020_TORRENT_PROJECT_TORRENTAPI_H
