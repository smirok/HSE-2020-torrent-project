#include <iostream>
#include <vector>
#include "gtest/gtest.h"

#include "TorrentAPI.h"
#include "Linker.h"

std::vector<std::pair<std::string,std::string>> torrent_pack = {
        {"/home/ilya/skachat_igru.torrent", "."},
};

TEST(DownloadByParts, check){
    TorrentAPI api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.pickDownloadFiles();
    api.picker.setMark(1,true);
    api.picker.setMark(6,true);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).state_ != "finished"){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
};

TEST(CreateDownload, start){
    TorrentAPI api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).state_ != "downloading"){
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).state_,"downloading");
}

TEST(CreateDownload, quarter_of_download){
    TorrentAPI api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download_ <= 25){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download_, 24);
}

TEST(CreateDownload, half_of_download){
    TorrentAPI api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download_ <= 50){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download_, 50);
}

TEST(CreateDownload, three_quarters_of_download){
    TorrentAPI api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download_ <= 75){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download_, 75);
}

TEST(CreateDownload, end_of_download){
    TorrentAPI api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).state_ != "finished"){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_EQ(api.getInfo(torrent_pack[0].first).state_,"finished");
}

TEST(makeTorrent, make_torrent) {
    TorrentAPI api;
    std::vector<std::string> trackers;
    trackers.emplace_back("google/com");
    api.makeTorrent("/home/ilya/for_testing/tester.txt",trackers,
                    "/home/ilya/for_testing", "myfile");
}