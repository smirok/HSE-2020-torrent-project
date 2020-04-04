#include <iostream>
#include <thread>

#include "API.h"
#include "InfoHelper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp> // unused
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp> // unused
#include <libtorrent/write_resume_data.hpp> // unused
#include <libtorrent/error_code.hpp>

API::API() {
    lt::settings_pack pack; // пакет выводимых команд для уведомлений о закаче
    pack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
                                                | lt::alert::storage_notification
                                                | lt::alert::status_notification);

    ses.apply_settings(pack); //добавляем пакет в сессию
}

void API::createDownload(const std::string &file_name, const std::string &path) {
    setFile(file_name);
    setPath(path);
    //p.flags |= lt::torrent_flags::seed_mode;

    ses.async_add_torrent(p);
    std::vector<lt::alert *> alerts;

    int get_add_alert = false;
    while (!get_add_alert) {
        ses.pop_alerts(&alerts);
        ses.post_torrent_updates();
        for (lt::alert const *a : alerts) {
            if (auto at = lt::alert_cast<lt::add_torrent_alert>(a)) {
                view.converter[file_name] = at->handle;
                view.session_handles[at->handle] = at->handle.status();
                get_add_alert = true;
                break;
            }
        }
    }

}

void API::removeDownload(const std::string &file_name, bool should_delete) {
    takeUpdates();

    lt::torrent_handle h = view.converter[file_name];

    auto i = view.session_handles.find(h);
    if (i == view.session_handles.end())
        return; // кто должен это обработать?

    auto j = std::find(view.current_handles.begin(),
            view.current_handles.end(),
            std::make_unique<lt::torrent_status>(i->second));

    if (j != view.current_handles.end())
        view.current_handles.erase(j);

    view.session_handles.erase(i);

    if (should_delete)
        ses.remove_torrent(h, lt::session_handle::delete_files); // потом
    else
        ses.remove_torrent(h);
}

void API::pauseDownload(const std::string &file_name) {
    takeUpdates();

    lt::torrent_handle h = view.converter[file_name];

    h.unset_flags(lt::torrent_flags::auto_managed);
    h.pause(lt::torrent_handle::graceful_pause);

    view.converter[file_name] = h;
}

void API::resumeDownload(const std::string &file_name) {
    takeUpdates();

    lt::torrent_handle h = view.converter[file_name];
    h.set_flags(lt::torrent_flags::auto_managed);

    view.converter[file_name] = h;
}

void API::setPath(const std::string &path) {
    p.save_path = path;
}

void API::setFile(const std::string &file_name) {
    p.ti = std::make_shared<lt::torrent_info>(file_name);
}

TorrentInfo API::getInfo(const std::string &file_name) {
    takeUpdates();

    TorrentInfo ti;
    ti.download_rate = ih.getDownloadRate(view.session_handles[view.converter[file_name]]);
    ti.state = ih.getState(view.session_handles[view.converter[file_name]].state);
    ti.remain_time = ih.getRemainTime(view.session_handles[view.converter[file_name]]);
    ti.total_size = ih.getTotalSize(view.session_handles[view.converter[file_name]]);
    ti.downloaded_size = ih.getDownloadedSize(view.session_handles[view.converter[file_name]]);
    ti.file_name = ih.getName(view.session_handles[view.converter[file_name]]);
    ti.percent_download = ih.getPercentDownloadedSize(view.session_handles[view.converter[file_name]]);

    return ti;
}

void API::takeUpdates() {
    std::vector<lt::alert *> alerts;
    ses.pop_alerts(&alerts);
    ses.post_torrent_updates();

    for (lt::alert const *a : alerts) {
        if (auto *pr = lt::alert_cast<lt::state_update_alert>(a)) {
            if (pr->status.empty())
                continue;
            view.updateTorrents(pr->status);
        }
    }
}

void API::makeTorrent(const std::string& path_to_files, const std::vector<std::string>& trackers) {
    lt::file_storage fs;

    add_files(fs, path_to_files);

    lt::create_torrent t(fs);
    for (auto& tracker : trackers)
        t.add_tracker(tracker);

    t.set_creator("OUR TORRENT PROJECT");

    //set_piece_hashes(t, ".");

    std::ofstream out("kek.torrent", std::ios::binary);
    bencode(std::ostream_iterator<char>(out), t.generate());
}
