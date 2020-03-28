#include "InfoHelper.h"

namespace Time {
    size_t DAY = 24 * 60 * 60;
    size_t HOUR = 24 * 60;
    size_t MINUTE = 60;
};


std::pair<double, std::string> InfoHelper::parseSize(int64_t byte_size) const {
    if (byte_size < (1 << 10))
        return {byte_size, "B"};
    if (byte_size < (1 << 20))
        return {byte_size / (1 << 10), "KB"};
    if (byte_size < (1 << 30))
        return {byte_size / (1 << 20), "MB"};
    return {byte_size / (1 << 30), "GB"};
}

std::string InfoHelper::endTime(int64_t remain, int64_t speed) const {
    std::string result = "";
    if (speed == 0)
        return "999d";
    if (remain >= speed * Time::DAY){
        result += std::to_string(remain / (speed * Time::DAY)) + "d";
        remain %= speed * Time::DAY;
    }
    if (remain >= speed * Time::HOUR){
        result += std::to_string(remain / (speed * Time::HOUR)) + "h";
        remain %= speed * Time::HOUR;
    }
    if (remain >= speed * Time::MINUTE){
        result += std::to_string(remain / (speed * Time::MINUTE)) + "m";
        remain %= speed * Time::MINUTE;
    }
    result += std::to_string(remain / speed) + "ses";
    return result;
}

std::string InfoHelper::getState(lt::torrent_status::state_t s) const {
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

std::string InfoHelper::getName(const lt::torrent_status &ts) const {
    return ts.name;
}

std::pair<long double, std::string> InfoHelper::getDownloadedSize(const lt::torrent_status &ts) const {
    return parseSize(ts.total_done);
}

std::pair<long double, std::string> InfoHelper::getTotalSize(const lt::torrent_status &ts) const {
    if (cachedSize.first == 0){
        cachedSize = parseSize(ts.total_done);
    }
    return cachedSize;
}

int InfoHelper::getPercentDownloadedSize(const lt::torrent_status &ts) const {
    return (ts.progress_ppm / (int)1e4); // занести в const
}

int InfoHelper::getDownloadRate(const lt::torrent_status &ts) const {
    return (ts.download_rate);
}

std::string InfoHelper::getRemainTime(const lt::torrent_status &ts) const {
    return endTime(ts.total - ts.total_done, ts.download_rate);
}