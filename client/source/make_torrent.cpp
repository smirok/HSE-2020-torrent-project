#include "make_torrent.h"

int main1() {
    lt::file_storage fs;

    add_files(fs, "/home/ilya/lildojd");

    lt::create_torrent t(fs);
    t.add_tracker("udp://192.168.0.107:8001");
    t.set_creator("HSE");

    set_piece_hashes(t, ".");

    std::ofstream out("HSE1.torrent", std::ios_base::binary);
    bencode(std::ostream_iterator<char>(out), t.generate());
    return 228;
}