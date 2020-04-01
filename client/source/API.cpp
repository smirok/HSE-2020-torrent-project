#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

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

    view.torrent_to_hash[file_name] = p.info_hash;
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
                //view.current_handles.push_back(at->handle.status());
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

}

void API::removeDownload(const std::string &file_name) {
    //lt::sha1_hash hash = view.torrent_to_hash[file_name];
    //lt::torrent_handle h = ses.find_torrent(hash);// --//-- и удалили скачку
    std::vector<lt::alert *> alerts;
    ses.pop_alerts(&alerts);
    ses.post_torrent_updates();
    for (lt::alert const *a : alerts) {
        if (auto *p = lt::alert_cast<lt::state_update_alert>(a))
            view.updateTorrents(p->status);
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    lt::torrent_handle h = view.converter[file_name];

    auto i = view.session_handles.find(h);
    if (i == view.session_handles.end()) {
        std::cout << " Warning! Torrent to remove not found! " << std::endl;
        return;
    }

    auto j = std::find(view.current_handles.begin(), view.current_handles.end(), &i->second);
    if (j != view.current_handles.end())
        view.current_handles.erase(j);

    view.session_handles.erase(i);

    // ========= это(ниже) мы делаем, а потом когда поймаем removed_alert, вызовем эту(выше) функцию=========
    ses.remove_torrent(h, lt::session_handle::delete_files); // потом
}

void API::pauseDownload(const std::string &file_name) {
    //lt::sha1_hash hash = view.torrent_to_hash[file_name];
    //ses.find_torrent(hash).pause(); // нашли торрент по хэшу и остановили!

    std::vector<lt::alert *> alerts;
    ses.pop_alerts(&alerts);
    ses.post_torrent_updates();
    for (lt::alert const *a : alerts) {
        if (auto *p = lt::alert_cast<lt::state_update_alert>(a))
            view.updateTorrents(p->status);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    lt::torrent_handle h = view.converter[file_name];

    lt::torrent_status const &ts = view.getTorrent(file_name);

    if ((ts.flags & (lt::torrent_flags::auto_managed
                     | lt::torrent_flags::paused)) == lt::torrent_flags::paused) {
        h.set_flags(lt::torrent_flags::auto_managed);
    } else {
        h.unset_flags(lt::torrent_flags::auto_managed);
        h.pause(lt::torrent_handle::graceful_pause);
    }

    view.converter[file_name] = h;
}

void API::resumeDownload(const std::string &file_name) {
    lt::sha1_hash hash = view.torrent_to_hash[file_name];
    ses.find_torrent(hash).resume(); // --//-- и продолжили скачку
}

void API::setPath(const std::string &path) {
    p.save_path = path;
}

void API::setFile(const std::string &file_name) {
    p.ti = std::make_shared<lt::torrent_info>(file_name);
}

void API::startDownload() try {
    ses.async_add_torrent(p); // формируем торрент скачку
    int is_done = false;

    for (;;) {
        std::vector<lt::alert *> alerts;
        ses.pop_alerts(&alerts);
        ses.post_torrent_updates();

        for (lt::alert const *a : alerts) {
            // finish or error - end!
            /*if (auto pa = lt::alert_cast<lt::torrent_paused_alert>(a)){
                std::cout << pa->torrent_name() << "Is paused" << std::endl;
                continue;
            }*/

            if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
                std::cout << a->message() << std::endl;
                is_done = true;
                break;
            }

            if (lt::alert_cast<lt::torrent_error_alert>(a)) {
                std::cout << a->message() << std::endl;
                is_done = true;
                break;
            }

            if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {
                if (st->status.empty()) continue;

                lt::torrent_status const &ts = st->status[0];

                std::cout << ts.total_done << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (is_done)
            break;
    }
    std::cout << "Download complete" << std::endl;
} catch (std::exception const &e) {
    std::cerr << "ERROR: " << e.what() << "\n";
}

TorrentInfo API::getInfo(const std::string &file_name) {
    std::vector<lt::alert *> alerts;
    ses.pop_alerts(&alerts);

    ses.post_torrent_updates();

    for (lt::alert const *a : alerts) {
        if (auto *pr = lt::alert_cast<lt::state_update_alert>(a)) {
            if (pr->status.empty())
                continue;
            for (auto &s : pr->status) {
                //view.session_handles[s.handle] = s;

                /*std::cout << "\r" << s.name << " " << ih.getState(s.state) << " "
                          << (s.download_payload_rate / 1000) << " kB/s "
                          << (s.total_done / 1000) << " kB ("
                          << (s.progress_ppm / 10000) << "%) downloaded\x1b[K" << std::endl;*/
                //std::cout.flush();
            }
            view.updateTorrents(pr->status);
            //std::cout << s.total_download << std::endl;
            //std::cout << std::endl;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

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
