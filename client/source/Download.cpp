#include "Download.h"
#include <string>
#include <iostream>
#include <pthread.h>


void Download::setPath(const std::string &path) {
    p.save_path = path;
}

void Download::setFile(const std::string &file_name) {
    p.ti = std::make_shared<lt::torrent_info>(file_name);
}

void Download::startDownload(lt::session &ses) try {
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