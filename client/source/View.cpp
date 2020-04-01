#include "View.h"

lt::torrent_status &View::getTorrent(const std::string& file_name) {
    return session_handles[converter[file_name]];
}

void View::updateTorrents(std::vector<lt::torrent_status> st) {
    bool need_update = false;
    for (lt::torrent_status& t : st){
        if (t.download_rate == 0)
            continue;
        auto j = session_handles.find(t.handle);
        if (j != session_handles.end()){
            auto handle = t.handle;
            session_handles[handle] = t;
            current_handles.push_back(&t);
            need_update = true;
        }
    }

    if (need_update)
        updateCurrentTorrents();
}

void View::updateCurrentTorrents() {
    current_handles.clear();
    for (auto &h : session_handles){
        if (h.first.is_valid())
            current_handles.push_back(&h.second);
    }

    for (auto i = current_handles.begin();
         i != current_handles.end();){
        lt::torrent_status const& s = **i;
        if (!s.handle.is_valid()){
            i = current_handles.erase(i);
            continue;
        }
        ++i;
    }
}
