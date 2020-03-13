#ifndef HSE_2020_TORRENT_PROJECT_CLIENT_H
#define HSE_2020_TORRENT_PROJECT_CLIENT_H

#include "info_helper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp> // unused
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp> // unused
#include <libtorrent/write_resume_data.hpp> // unused
#include <libtorrent/error_code.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/extensions/smart_ban.hpp>

class client{
public:
    client();
    void init();
    void set_path(const std::string& path);
    void set_file(const std::string& file_name);
    info_helper ih;

    lt::torrent_status st;
    lt::add_torrent_params p; // будут дефолтные пока
    lt::session ses;

    void print_download_log(const lt::torrent_status &ts,
                            std::pair<long double, std::string> &full,
                            std::string &remain_time);
};


#endif //HSE_2020_TORRENT_PROJECT_CLIENT_H
