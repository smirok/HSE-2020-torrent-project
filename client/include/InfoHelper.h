#ifndef HSE_2020_TORRENT_PROJECT_INFOHELPER_H
#define HSE_2020_TORRENT_PROJECT_INFOHELPER_H

#include <string>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include "libtorrent/torrent_status.hpp"

class InfoHelper {
public:
    // ====================HELP FUNCTIONS==================

    std::pair<double, std::string> parseSize(uint64_t byte_size) const noexcept;
    std::string endTime(int64_t remain, int64_t speed) const noexcept;

    // ====================GUI-getters=====================

    std::string getState(const lt::torrent_status::state_t& s) const noexcept;
    std::string getName(const lt::torrent_status &ts) const noexcept;
    std::pair<long double, std::string> getDownloadedSize(const lt::torrent_status &ts) const noexcept;
    std::pair<long double, std::string> getTotalSize(const lt::torrent_status &ts) noexcept;
    uint32_t getPercentDownloadedSize(const lt::torrent_status &ts) const noexcept;
    uint32_t getDownloadRate(const lt::torrent_status &ts) const noexcept;
    std::string getRemainTime(const lt::torrent_status &ts) const noexcept;

private:
    std::unordered_map<std::string, std::pair<long double, std::string>> cachedTotalSize;
};

struct TorrentInfo {
    std::string remain_time;
    uint64_t download_rate;
    uint32_t percent_download;
    std::string file_name;
    std::pair<long double, std::string> total_size;
    std::pair<long double, std::string> downloaded_size;
    std::string state;

    friend std::ostream &operator<<(std::ostream &out, const TorrentInfo &ti) {
        out << ti.state << " : " << ti.file_name <<
                  "   " << ti.downloaded_size.first << ti.downloaded_size.second << " of "
                  << ti.total_size.first << ti.total_size.second << "\n";
        out << ti.download_rate << " " << ti.percent_download << " " << ti.remain_time << "\n";
        return out;
    }
};


#endif //HSE_2020_TORRENT_PROJECT_INFOHELPER_H
