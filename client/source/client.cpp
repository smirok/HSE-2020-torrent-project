#include "client.h"
#include <string>
#include <iostream>

void client::init() {
    lt::settings_pack pack; // пакет выводимых команд для уведомлений о закаче
    pack.set_int(lt::settings_pack::alert_mask, lt::alert::error_notification
                                                | lt::alert::storage_notification
                                                | lt::alert::status_notification);

    ses.apply_settings(pack); //добавляем пакет в сессию
    ses.add_extension(&lt::create_smart_ban_plugin); // баним пиров, которые часто посылают плохие куски
}

client::client() {
    init();
}

void client::set_path(const std::string &path) {
    p.save_path = path;
}

void client::set_file(const std::string &file_name) {
    p.ti = std::make_shared<lt::torrent_info>(file_name);
}

void client::print_download_log(const lt::torrent_status &ts,
                        std::pair<long double, std::string> &full, std::string &remain_time) {
    std::cout << "\r" << ih.state(ts.state) << " " << "\n" // state

              << ih.parse_size(ts.total_done).first << ih.parse_size(ts.total_done).second << " of "
              << full.first << full.second << " downloaded "
              << "(" << (ts.progress_ppm / 10000) << "%)\n"   // сколько скачалось

              << (ts.download_payload_rate / 1000) << " kB/ses \n" // скорость

              << "End time " << remain_time << "\n" // осталось времени
              << std::endl;
}



