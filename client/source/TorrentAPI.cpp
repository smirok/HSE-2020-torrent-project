#include <string>
#include "TorrentAPI.h"
#include "InfoHelper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/download_priority.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/fwd.hpp>
#include "libtorrent/entry.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/create_torrent.hpp"

TorrentAPI::TorrentAPI() {
    lt::settings_pack pack; // пакет выводимых команд для уведомлений о закаче
    pack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
                                                | lt::alert::storage_notification
                                                | lt::alert::status_notification);

    session_.apply_settings(pack);
}

void TorrentAPI::prepareDownload(const std::string &file_name, const std::string &path) {
    picker.download_holder.clear();
    setFile(file_name);
    setPath(path);
}

void TorrentAPI::pickDownloadFiles() {
    lt::file_storage const &files = params_.ti->files();
    std::string sep = "/";

    std::unordered_map<std::string, int32_t> is_in_tree;
    std::unordered_map<int32_t, std::string> number_to_name;
    std::set<int32_t> tree[FilesPicker::MAX_FILES];
    std::vector<uint64_t> files_size;

    int32_t current_number = 1;

    for (auto iter : files.file_range()) {

        std::string temp = files.file_path(iter);
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
                files_size.push_back(files.file_size(iter));
                break;
            }

            name = name.substr(pos + sep.size(), FilesPicker::MAX_LENGTH);
        }
    }

    info_helper_.dfs(picker.download_holder, tree, number_to_name, files_size, FilesPicker::ROOT, 0);
    info_helper_.recalcSize(picker.download_holder, tree, FilesPicker::ROOT);
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
            if (!mark) {
                download_holder[current_index].is_marked_ = mark;
            } else {
                uint32_t temp_index = current_index + 1;
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

void TorrentAPI::createDownload(const std::string &file_name) {
    session_.async_add_torrent(params_);
    std::vector<lt::alert *> alerts;

    bool get_add_alert = false;

    while (!get_add_alert) {

        session_.pop_alerts(&alerts);
        session_.post_torrent_updates();

        for (const auto &alert : alerts) {
            if (const auto &add_alert = lt::alert_cast<lt::add_torrent_alert>(alert)) {
                linker_.converter[file_name] = add_alert->handle;
                linker_.session_handles[add_alert->handle] = add_alert->handle.status();
                get_add_alert = true;

                int32_t current_file = 0;
                for (const auto &object : picker.download_holder) {
                    if (object.is_leaf_)
                        linker_.converter[file_name].file_priority(current_file++,
                                                                  object.is_marked_ ? lt::default_priority
                                                                                    : lt::dont_download);
                }
                break;
            }
        }
    }
}

void TorrentAPI::removeDownload(const std::string &file_name, bool should_delete) {
    takeUpdates();

    lt::torrent_handle handle = linker_.converter[file_name];

    auto handle_iter = linker_.session_handles.find(handle);
    if (handle_iter == linker_.session_handles.end())
        return;

    auto current_handle_iter = std::find(linker_.current_handles.begin(),
                                         linker_.current_handles.end(),
                                         std::make_unique<lt::torrent_status>(handle_iter->second));

    if (current_handle_iter != linker_.current_handles.end())
        linker_.current_handles.erase(current_handle_iter);

    linker_.session_handles.erase(handle_iter);

    if (should_delete)
        session_.remove_torrent(handle, lt::session_handle::delete_files); // потом
    else
        session_.remove_torrent(handle);
}

void TorrentAPI::pauseDownload(const std::string &file_name) {
    takeUpdates();

    lt::torrent_handle handle = linker_.converter[file_name];

    handle.unset_flags(lt::torrent_flags::auto_managed);
    handle.pause(lt::torrent_handle::graceful_pause);

    linker_.converter[file_name] = handle;
}

void TorrentAPI::resumeDownload(const std::string &file_name) {
    takeUpdates();

    lt::torrent_handle handle = linker_.converter[file_name];
    handle.set_flags(lt::torrent_flags::auto_managed);

    linker_.converter[file_name] = handle;
}

void TorrentAPI::setPath(const std::string &path) {
    params_.save_path = path;
}

void TorrentAPI::setFile(const std::string &file_identifier) {
    if (file_identifier.substr(0, 6) == "magnet")
        params_ = lt::parse_magnet_uri(file_identifier);
    else {
        params_.ti = std::make_shared<lt::torrent_info>(file_identifier);
        for (int i = 0; i < params_.ti->num_files(); ++i)
            params_.file_priorities.push_back(lt::default_priority);
    }
}

TorrentInfo TorrentAPI::getInfo(const std::string &file_name) {
    takeUpdates();

    TorrentInfo ti;

    ti.download_rate_ = info_helper_.getDownloadRate(linker_.session_handles[linker_.converter[file_name]]);
    ti.state_ = info_helper_.getState(linker_.session_handles[linker_.converter[file_name]].state);
    ti.remain_time_ = info_helper_.getRemainTime(linker_.session_handles[linker_.converter[file_name]]);
    ti.total_size_ = info_helper_.getTotalSize(linker_.session_handles[linker_.converter[file_name]]);
    ti.downloaded_size_ = info_helper_.getDownloadedSize(linker_.session_handles[linker_.converter[file_name]]);
    ti.file_name_ = info_helper_.getName(linker_.session_handles[linker_.converter[file_name]]);
    ti.percent_download_ = info_helper_.getPercentDownloadedSize(linker_.session_handles[linker_.converter[file_name]]);

    std::string double_downloaded_size = std::to_string(round(ti.downloaded_size_.first * 100) / 100);
    std::string double_total_size = std::to_string(round(ti.total_size_.first * 100) / 100);
    for (int i = 0; i < 4; ++i) {
        double_downloaded_size.pop_back();
        double_total_size.pop_back();
    }

    ti.progress_info_ = double_downloaded_size + ti.downloaded_size_.second + " of " +
                        double_total_size + ti.total_size_.second;

    return ti;
}

void TorrentAPI::takeUpdates() {
    std::vector<lt::alert *> alerts;
    session_.pop_alerts(&alerts);
    session_.post_torrent_updates();

    for (const auto &alert : alerts) {
        if (const auto &update_alert = lt::alert_cast<lt::state_update_alert>(alert)) { // fix ptrs
            if (update_alert->status.empty())
                continue;
            linker_.updateTorrents(update_alert->status);
        }
    }
}

void TorrentAPI::makeTorrent(const std::string &path_to_files,
                             const std::vector<std::string> &trackers,
                             const std::string &path_to_torrent_file,
                             const std::string &torrent_file_name) {

    lt::file_storage fstorage;
    add_files(fstorage, path_to_files);

    lt::create_torrent creator(fstorage);
    for (auto &tracker : trackers)
        creator.add_tracker(tracker);

    creator.set_creator("TorrentX");
    std::string full_path_torrent_file = path_to_torrent_file + "/" + torrent_file_name + ".torrent";

    std::ofstream out(full_path_torrent_file, std::ios::binary);
    bencode(std::ostream_iterator<char>(out), creator.generate());
}