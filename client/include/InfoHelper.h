#ifndef HSE_2020_TORRENT_PROJECT_INFOHELPER_H
#define HSE_2020_TORRENT_PROJECT_INFOHELPER_H

#include <string>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <utility>
#include "libtorrent/torrent_status.hpp"

struct FileNode {
    FileNode(std::string name, int32_t level, bool is_marked, bool is_leaf) :
            name_(std::move(name)), level_(level), is_marked_(is_marked), is_leaf_(is_leaf) {
    }

    std::string name_;
    int32_t level_;
    bool is_marked_;
    bool is_leaf_ = false;
    uint64_t fullsize_ = 0;
};

class InfoHelper {
public:
    // ====================HELP FUNCTIONS==================

    std::pair<long double, std::string> parseSize(long double byte_size) const noexcept; // uin64_t

    std::string endTime(int64_t remain, int64_t speed) const noexcept;

    void dfs(std::vector<FileNode> &result, std::set<int32_t> *tree,
             std::unordered_map<int32_t, std::string> &conv,
             int32_t vertex, int32_t lvl, int32_t parent = -1);

    void recalcSize(std::vector<FileNode> &result,
                    std::set<int32_t> *tree,
                    int32_t vertex, int32_t parent = -1);

    // ====================GUI-getters=====================

    std::string getState(const lt::torrent_status::state_t &s) const noexcept;

    std::string getName(const lt::torrent_status &ts) const noexcept;

    std::pair<long double, std::string> getDownloadedSize(const lt::torrent_status &ts) const noexcept;

    std::pair<long double, std::string> getTotalSize(const lt::torrent_status &ts) noexcept;

    uint32_t getPercentDownloadedSize(const lt::torrent_status &ts) const noexcept;

    uint32_t getDownloadRate(const lt::torrent_status &ts) const noexcept;

    std::string getRemainTime(const lt::torrent_status &ts) const noexcept;

};

struct TorrentInfo {

    std::string remain_time_;
    uint64_t download_rate_;
    uint32_t percent_download_;
    std::string file_name_;
    std::pair<long double, std::string> total_size_;
    std::pair<long double, std::string> downloaded_size_;
    std::string state_;
    std::string progress_info_;

    friend std::ostream &operator<<(std::ostream &out, const TorrentInfo &ti) {
        out << ti.state_ << " : " << ti.file_name_ <<
            "   " << ti.progress_info_ << '\n';
        out << ti.download_rate_ << " " << ti.percent_download_ << " " << ti.remain_time_ << "\n";
        return out;
    }
};

#endif //HSE_2020_TORRENT_PROJECT_INFOHELPER_H
