#ifndef HSE_2020_TORRENT_PROJECT_API_H
#define HSE_2020_TORRENT_PROJECT_API_H

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include <iostream>

void enter_dir_and_file(lt::add_torrent_params & p, const std::string& file, const std::string& path);

void start_download(const std::string& path, const std::string& file);

char const *state(lt::torrent_status::state_t s);


#endif //HSE_2020_TORRENT_PROJECT_API_H
