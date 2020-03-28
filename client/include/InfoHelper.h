#ifndef HSE_2020_TORRENT_PROJECT_INFOHELPER_H
#define HSE_2020_TORRENT_PROJECT_INFOHELPER_H

#include <string>
#include <cstdint>
#include "libtorrent/torrent_status.hpp"

// namespace time
// set_flags : seed_mode
// фабрика торрентов

class InfoHelper {
public:
    // ====================HELP FUNCTIONS==================

    std::pair<double, std::string> parseSize(int64_t byte_size) const;

    std::string endTime(int64_t remain, int64_t speed) const;

    // ====================GUI-getters=====================

    std::string getState(lt::torrent_status::state_t s) const;

    std::string getName(const lt::torrent_status &ts) const;

    std::pair<long double, std::string> getDownloadedSize(const lt::torrent_status &ts) const;

    std::pair<long double, std::string> getTotalSize(const lt::torrent_status &ts) const;

    int getPercentDownloadedSize(const lt::torrent_status &ts) const;

    int getDownloadRate(const lt::torrent_status &ts) const;

    std::string getRemainTime(const lt::torrent_status &ts) const;

private:
    mutable std::pair<long double, std::string> cachedSize{0, ""}; // пара из размера и величины размера(b,Kb,Mb,..)

};


#endif //HSE_2020_TORRENT_PROJECT_INFOHELPER_H
