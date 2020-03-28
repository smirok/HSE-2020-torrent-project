#ifndef HSE_2020_TORRENT_PROJECT_DOWNLOAD_H
#define HSE_2020_TORRENT_PROJECT_DOWNLOAD_H

#include "InfoHelper.h"

#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp> // unused
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp> // unused
#include <libtorrent/write_resume_data.hpp> // unused
#include <libtorrent/error_code.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/extensions/smart_ban.hpp>



#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <unistd.h>

class Download {
public:

    void startDownload(lt::session &ses);

    void setPath(const std::string &path);

    void setFile(const std::string &file_name);

    lt::add_torrent_params p; // будут дефолтные пока
};

// ==========TO KEYBOARD=============

int kbhit();

#endif //HSE_2020_TORRENT_PROJECT_DOWNLOAD_H
