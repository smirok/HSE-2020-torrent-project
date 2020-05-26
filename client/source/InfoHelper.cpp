#include "InfoHelper.h"

namespace Time {
    constexpr int64_t DAY = 24 * 60 * 60;
    constexpr int64_t HOUR = 60 * 60;
    constexpr int64_t MINUTE = 60;
};

namespace MeasureConstants {
    constexpr uint64_t BYTES_IN_KB = 1'000; //  1 << 10
    constexpr uint64_t BYTES_IN_MB = 1'000'000; // 1 << 20
    constexpr uint64_t BYTES_IN_GB = 1'000'000'000; // 1 << 30
    constexpr int32_t CONVERT_LT_PPM = 1e4;
}


std::pair<long double, std::string> InfoHelper::parseSize(long double byte_size) const noexcept {
    if (byte_size < MeasureConstants::BYTES_IN_KB)
        return {byte_size, "B"};
    if (byte_size < MeasureConstants::BYTES_IN_MB)
        return {byte_size / MeasureConstants::BYTES_IN_KB, "KB"};
    if (byte_size < MeasureConstants::BYTES_IN_GB)
        return {byte_size / MeasureConstants::BYTES_IN_MB, "MB"};
    return {byte_size / MeasureConstants::BYTES_IN_GB, "GB"};
}

std::string InfoHelper::endTime(int64_t remain, int64_t speed) const noexcept {
    std::string result;
    if (speed == 0)
        return "999d";
    if (remain >= speed * Time::DAY) {
        result += std::to_string(remain / (speed * Time::DAY)) + "d";
        remain %= speed * Time::DAY;
    }
    if (remain >= speed * Time::HOUR) {
        result += std::to_string(remain / (speed * Time::HOUR)) + "h";
        remain %= speed * Time::HOUR;
    }
    if (remain >= speed * Time::MINUTE) {
        result += std::to_string(remain / (speed * Time::MINUTE)) + "m";
        remain %= speed * Time::MINUTE;
    }
    result += std::to_string(remain / speed) + "sec";
    return result;
}

std::string InfoHelper::getState(const lt::torrent_status::state_t &s) const noexcept {
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

std::string InfoHelper::getName(const lt::torrent_status &ts) const noexcept {
    return ts.name;
}

std::pair<long double, std::string> InfoHelper::getDownloadedSize(const lt::torrent_status &ts) const noexcept {
    return parseSize(ts.total_done);
}

std::pair<long double, std::string> InfoHelper::getTotalSize(const lt::torrent_status &ts) noexcept {
    return parseSize(ts.total_wanted);
    //std::cout << ts.total_wanted << "\n";
    //return {ts.total_wanted, "kek"};
}

uint32_t InfoHelper::getPercentDownloadedSize(const lt::torrent_status &ts) const noexcept {
    return (ts.progress_ppm / MeasureConstants::CONVERT_LT_PPM);
}

uint32_t InfoHelper::getDownloadRate(const lt::torrent_status &ts) const noexcept {
    return (ts.download_rate);
}

std::string InfoHelper::getRemainTime(const lt::torrent_status &ts) const noexcept {
    return endTime(ts.total_wanted - ts.total_done, ts.download_rate);
}

void InfoHelper::dfs(std::vector<FileNode> &result, std::set<int32_t> *tree,
                     std::unordered_map<int32_t, std::string> &conv,
                     std::vector<uint64_t> &files_size,
                     int32_t vertex, int32_t lvl, int32_t parent) {
    if (parent != -1) {
        result.emplace_back(conv[vertex], lvl, 0, tree[vertex].empty() ? true : false);
        if (tree[vertex].empty()) {
            result.back().fullsize_ = files_size[0];
            files_size.erase(files_size.begin());
        }
    }
    for (auto child : tree[vertex])
        if (child != parent)
            dfs(result, tree, conv, files_size, child, lvl + 1, vertex);
}

void InfoHelper::recalc_size(std::vector<FileNode> &result,
                             std::set<int32_t> *tree,
                             int32_t vertex,
                             int32_t parent) {
    for (auto child : tree[vertex])
        if (child != parent) {
            recalc_size(result, tree, child, vertex);
        }
    if (parent != -1 && parent - 1 >= 0)
        result[parent - 1].fullsize_ += result[vertex - 1].fullsize_;
}
