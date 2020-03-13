#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include "API.h"
#include "info_helper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp> // unused
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp> // unused
#include <libtorrent/write_resume_data.hpp> // unused
#include <libtorrent/error_code.hpp>


void API::enter_file(const std::string &file) {
    cl.set_file(file);
}

void API::enter_dir(const std::string &path) {
    cl.set_path(path);
}

void API::start_download() try {
    cl.ses.async_add_torrent(cl.p); // формируем торрент скачку

    for (;;) {
        std::vector<lt::alert *> alerts;
        cl.ses.pop_alerts(&alerts);
        cl.ses.post_torrent_updates();

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

                std::string remain_time = cl.ih.end_time(ts.total - ts.total_done, ts.download_payload_rate);
                std::pair<long double, std::string> full = cl.ih.parse_size(ts.total);

                cl.print_download_log(ts,full,remain_time);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    done:
    std::cout << "Download complete" << std::endl;
} catch (std::exception const &e) {
    std::cerr << "ERROR: " << e.what() << "\n";
}
