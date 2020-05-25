#include <iostream>
#include <string>
#include<thread>
#include "API.h"
#include "InfoHelper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp> // unused
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp> // unused
#include <libtorrent/write_resume_data.hpp> // unused
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/download_priority.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/fwd.hpp>

#include "libtorrent/entry.hpp"
#include "libtorrent/torrent_info.hpp"

API::API() {
    lt::settings_pack pack; // пакет выводимых команд для уведомлений о закаче
    pack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
                                                | lt::alert::storage_notification
                                                | lt::alert::status_notification);

    ses.apply_settings(pack);
}

void API::prepareDownload(const std::string &file_name, const std::string &path) {
    setFile(file_name);
    setPath(path);
}

void API::pickDownloadFiles() {

    lt::file_storage const &files = p.ti->files();
    std::string sep = "/";

    std::unordered_map<std::string, int32_t> is_in_tree;
    std::unordered_map<int32_t, std::string> number_to_name;
    std::set<int32_t> tree[picker.MAX_FILES];
    std::vector<uint64_t> files_size;

    int32_t current_number = 1;

    for (auto i : files.file_range()) {
        std::string temp = files.file_path(i);
        std::string name = temp;
        uint32_t parent_number = 0;
        while (true) {
            auto pos = name.find(sep);

            std::string path_part = name.substr(0, pos);
            if (!is_in_tree[path_part]) {
                is_in_tree[path_part] = current_number++;
                tree[parent_number].insert(is_in_tree[path_part]);
                number_to_name[current_number - 1] = path_part;
            }
            parent_number = is_in_tree[path_part];

            if (pos == std::string::npos) {
                files_size.push_back(files.file_size(i));
                break;
            }

            name = name.substr(pos + sep.size(), picker.MAX_LENGTH);
        }
    }

    ih.dfs(picker.download_holder, tree, number_to_name, files_size, 0, 0);
    ih.recalc_size(picker.download_holder, tree, 0);
}

void FilesPicker::setMark(int32_t index, bool mark) {
    download_holder[index].is_marked_ = mark;
    int32_t picked_level = download_holder[index].level_;
    int32_t current_index = index - 1;

    while (download_holder[++index].level_ > picked_level &&
           static_cast<unsigned long>(index) < download_holder.size()) {
        download_holder[index].is_marked_ = mark;
    }

    while (current_index >= 0) {
        if (download_holder[current_index].level_ < picked_level) {
            if (!mark)
                download_holder[current_index].is_marked_ = mark;
            else {
                uint32_t temp_index = current_index+1;
                uint32_t subtree_flags = 0;
                while (temp_index < download_holder.size() &&
                        download_holder[temp_index].level_ > download_holder[current_index].level_) {
                    if (download_holder[temp_index++].is_marked_)
                        subtree_flags++;
                }
                if (subtree_flags == temp_index - current_index - 1)
                    download_holder[current_index].is_marked_ = mark;
            }
            picked_level = download_holder[current_index].level_;
        }
        --current_index;
    }
}

std::vector<bool> FilesPicker::getMarks() {
    std::vector<bool> result;

    for (const auto &object : download_holder) {
        result.push_back(object.is_marked_);
    }

    return result;
}

void API::createDownload(const std::string &file_name) {
    //p.flags |= lt::torrent_flags::seed_mode; - где сидить?

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

                int32_t current_file = 0;
                for (const auto &object : picker.download_holder) {
                    if (object.is_leaf_)
                        view.converter[file_name].file_priority(current_file++,
                                                                object.is_marked_ ? lt::default_priority
                                                                                  : lt::dont_download);
                }
                picker.download_holder.clear();
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

void API::setFile(const std::string &file_identifier) {
    if (file_identifier.substr(0, 6) == "magnet")
        p = lt::parse_magnet_uri(file_identifier);
    else {
        p.ti = std::make_shared<lt::torrent_info>(file_identifier);
        p.file_priorities.push_back(lt::dont_download);
        p.file_priorities.push_back(lt::dont_download);
        p.file_priorities.push_back(lt::default_priority);
        p.file_priorities.push_back(lt::dont_download);
        p.file_priorities.push_back(lt::dont_download);
        p.file_priorities.push_back(lt::dont_download);
    }
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
    ti.progress_info = std::to_string(static_cast<uint64_t>(ti.downloaded_size.first)) + ti.downloaded_size.second + " of " +
            std::to_string(static_cast<uint64_t>(ti.total_size.first)) + ti.total_size.second;

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

void API::makeTorrent(const std::string &path_to_files,
                      const std::vector<std::string> &trackers,
                      const std::string &path_to_torrent_file,
                      const std::string &torrent_file_name) {
    lt::file_storage fs;
    add_files(fs, path_to_files);

    lt::create_torrent t(fs);
    for (auto &tracker : trackers)
        t.add_tracker(tracker);

    t.set_creator("TorrentX");
    std::string full_path_torrent_file = path_to_torrent_file + "/" + torrent_file_name + ".torrent";

    std::ofstream out(full_path_torrent_file, std::ios::binary);
    bencode(std::ostream_iterator<char>(out), t.generate());
}