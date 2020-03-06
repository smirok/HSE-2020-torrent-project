#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include "../include/API.h"
#include "../include/info_helper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp> // unused
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp> // unused
#include <libtorrent/write_resume_data.hpp> // unused
#include <libtorrent/error_code.hpp>

void enter_dir_and_file(lt::add_torrent_params &p, const std::string& file, const std::string& path) {
    p.save_path = path;
    p.ti = std::make_shared<lt::torrent_info>(file); // формируем инфу о файле
}

char const *state(lt::torrent_status::state_t s) {
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

void start_download(const std::string& path, const std::string& file) {

    try {
        lt::settings_pack pack; // пакет выводимых команд для уведомлений о закаче
        pack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
                                                    | lt::alert::storage_notification
                                                    | lt::alert::status_notification);

        lt::session s(pack); // добавляем пакет в сессию

        lt::add_torrent_params p; // генерим дефолтные параметры
        enter_dir_and_file(p, file, path); // API : вводим директорию и путь до .torrent
        s.add_torrent(p); // формируем торрент скачку

        for (;;) {
            std::vector<lt::alert *> alerts;
            s.pop_alerts(&alerts);
            s.post_torrent_updates();

            for (lt::alert const *a : alerts) {
                // finish or error - end!
                if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
                    std::cout << a->message() << std::endl;
                    goto done;
                }
                if (lt::alert_cast<lt::torrent_error_alert>(a)) {
                    std::cout << a->message() << std::endl;
                    goto done;
                }
                if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {
                    if (st->status.empty()) continue;

                    lt::torrent_status const &ts = st->status[0];

                    std::string remain_time = fi::end_time(ts.total - ts.total_done, ts.download_payload_rate);
                    std::pair<long double, std::string> full = fi::parse_size(ts.total);

                    std::cout << "\r" << state(ts.state) << " " << "\n" // state

                              << fi::parse_size(ts.total_done).first << fi::parse_size(ts.total_done).second << " of "
                              << full.first << full.second << " downloaded "
                              << "(" << (ts.progress_ppm / 10000) << "%)\n"   // сколько скачалось

                              << (ts.download_payload_rate / 1000) << " kB/s \n" // скорость

                              << "End time " << remain_time << "\n" // осталось времени
                              << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        done:
        std::cout << "Download complete" << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}