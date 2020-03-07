#ifndef HSE_2020_TORRENT_PROJECT_INFO_HELPER_H
#define HSE_2020_TORRENT_PROJECT_INFO_HELPER_H

#include <string>
#include "cstdint"
#include "libtorrent/torrent_status.hpp"

enum time {
    DAY = 24 * 60 * 60,
    HOUR = 24 * 60,
    MINUTE = 60,
};

class info_helper {
public:
    std::pair<long double, std::string> parse_size(std::int64_t byte_size);

    std::string end_time(std::int64_t remain, std::int64_t speed);

    char const *state(lt::torrent_status::state_t s);
};

#endif //HSE_2020_TORRENT_PROJECT_INFO_HELPER_H
