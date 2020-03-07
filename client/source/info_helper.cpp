#include "info_helper.h"

std::pair<long double, std::string> info_helper::parse_size(std::int64_t byte_size) {
    if (byte_size < (1 << 10))
        return {byte_size, "B"};
    if (byte_size < (1 << 20))
        return {(long double) byte_size / (1 << 10), "KB"};
    if (byte_size < (1 << 30))
        return {(long double) byte_size / (1 << 20), "MB"};
    return {(long double) byte_size / (1 << 30), "GB"};
}

std::string info_helper::end_time(std::int64_t remain, std::int64_t speed) {
    std::string result = "";
    if (speed == 0)
        return "999d";
    if (remain >= speed * time::DAY){
        result += std::to_string(remain / (speed * time::DAY)) + "d";
        remain %= speed * time::DAY;
    }
    if (remain >= speed * time::HOUR){
        result += std::to_string(remain / (speed * time::HOUR)) + "h";
        remain %= speed * time::HOUR;
    }
    if (remain >= speed * time::MINUTE){
        result += std::to_string(remain / (speed * time::MINUTE)) + "m";
        remain %= speed * time::MINUTE;
    }
    result += std::to_string(remain / speed) + "s";
    return result;
}

const char * info_helper::state(lt::torrent_status::state_t s) {
    switch (s) {
        case lt::torrent_status::checking_files:
            return "checking";
        case lt::torrent_status::downloading_metadata:
            return "dl metadata";
        case lt::torrent_status::downloading:
            return "downloading";
        case lt::torrent_status::finished:
            return "finished";
        case lt::torrent_status::seeding:
            return "seeding";
        case lt::torrent_status::allocating:
            return "allocating";
        case lt::torrent_status::checking_resume_data:
            return "checking resume";
        default:
            return "<>";
    }
}