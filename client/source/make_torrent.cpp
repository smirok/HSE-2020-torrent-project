#include "make_torrent.h"

int main(){
        lt::file_storage fs;

        add_files(fs, "/home/ilya/CLionProjects/contest");

        lt::create_torrent t(fs);
        t.add_tracker("http://my.tracker.com/announce");
        t.set_creator("HSE");

        //set_piece_hashes(t, ".");

        std::ofstream out("HSE.torrent", std::ios_base::binary);
        bencode(std::ostream_iterator<char>(out), t.generate());
}