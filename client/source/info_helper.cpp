#include "../include/info_helper.h"

std::pair<long double, std::string> fi::parse_size(std::int64_t byte_size) {
    if (byte_size < (1 << 10))
        return {byte_size, "B"};
    if (byte_size < (1 << 20))
        return {(long double) byte_size / (1 << 10), "KB"};
    if (byte_size < (1 << 30))
        return {(long double) byte_size / (1 << 20), "MB"};
    return {(long double) byte_size / (1 << 30), "GB"};
}

std::string fi::end_time(std::int64_t remain, std::int64_t speed){
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