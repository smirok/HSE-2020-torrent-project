#include "Linker.h"

void Linker::updateTorrents(const std::vector<lt::torrent_status> &statuses) {
    bool need_update = false;
    for (const auto &status : statuses) {
        if (status.download_rate == 0)
            continue;
        auto updatable_status = session_handles.find(status.handle);
        if (updatable_status != session_handles.end()) {
            auto handle = status.handle;
            session_handles[handle] = status;
            current_handles.push_back(std::make_unique<lt::torrent_status const>(status));
            need_update = true;
        }
    }

    if (need_update)
        updateCurrentTorrents();
}

void Linker::updateCurrentTorrents() {
    current_handles.clear();
    for (const auto &handle : session_handles) {
        if (handle.first.is_valid())
            current_handles.push_back(std::make_unique<lt::torrent_status const>(handle.second));
    }

    for (auto iter = current_handles.begin(); iter != current_handles.end();) {
        lt::torrent_status const &status = **iter;
        if (!status.handle.is_valid()) {
            iter = current_handles.erase(iter);
            continue;
        }
        ++iter;
    }
}