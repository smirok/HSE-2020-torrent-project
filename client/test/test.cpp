#include <iostream>
#include <vector>
#include "gtest/gtest.h"

#include "API.h"
#include "View.h"

std::vector<std::pair<std::string,std::string>> torrent_pack = {
        {"/home/ilya/skachat_igru_bad_company.torrent", "."},
};

TEST(DownloadByParts, check){
    API api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.pickDownloadFiles();
    api.picker.setMark(0,true);
    api.picker.setMark(6,true);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 20){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        //std::cout << api.getInfo(torrent_pack[0].first).total_size.first << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    api.pauseDownload(torrent_pack[0].first);
    while (true) {
        std::cout << api.getInfo(torrent_pack[0].first);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    exit(0);
};

TEST(CreateDownload, start){
    API api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).state != "downloading"){
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).state,"downloading");
}

TEST(CreateDownload, quarter_of_download){
    API api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 25){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download, 24);
}

TEST(CreateDownload, half_of_download){
    API api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 50){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download, 50);
}

TEST(CreateDownload, three_quarters_of_download){
    API api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 75){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download, 75);
}

TEST(CreateDownload, end_of_download){
    API api;
    api.prepareDownload(torrent_pack[0].first,torrent_pack[0].second);
    api.createDownload(torrent_pack[0].first);
    while (api.getInfo(torrent_pack[0].first).state != "finished"){
        std::cout << api.getInfo(torrent_pack[0].first) << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_EQ(api.getInfo(torrent_pack[0].first).state,"finished");
}

TEST(makeTorrent, make_torrent) {
    API api;
    std::vector<std::string> trackers;
    trackers.emplace_back("google/com");
    api.makeTorrent("/home/ilya/for_testing/tester.txt",trackers,
                    "/home/ilya/for_testing", "myfile");
}