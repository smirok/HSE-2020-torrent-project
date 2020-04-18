#include <iostream>
#include <vector>
#include "gtest/gtest.h"

#include "API.h"
#include "InfoHelper.h"
#include "View.h"

std::vector<std::pair<std::string,std::string>> torrent_pack = {
        {"/home/ilya/Game_Dev_Tycoon.torrent", "."},
        {"/home/ilya/Hobo_Tough_Life_v0.84.exe.torrent", "."},
        {"/home/ilya/348_Oboi-devushki_0.torrent", "."}
};

TEST(CreateDownload, start){
    //freopen("output.txt","w",stdout);
    API api;
    api.createDownload(torrent_pack[0].first,torrent_pack[0].second);
    while (api.getInfo(torrent_pack[0].first).state != "downloading"){
        //std::cout << api.getInfo(torrent_pack[2].first);
        //std::cout << "kek\n";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).state,"downloading");
}

TEST(CreateDownload, quarter_of_download){
    //freopen("output.txt","w",stdout);
    API api;
    api.createDownload(torrent_pack[0].first,torrent_pack[0].second);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 25){
        //std::cout << api.getInfo(torrent_pack[2].first);
        //std::cout << "kek\n";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "keker";
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download, 24);
}

TEST(CreateDownload, half_of_download){
    //freopen("output.txt","w",stdout);
    API api;
    api.createDownload(torrent_pack[0].first,torrent_pack[0].second);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 50){
        //std::cout << api.getInfo(torrent_pack[2].first);
        //std::cout << "kek\n";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download, 50);
}

TEST(CreateDownload, three_quarters_of_download){
    //freopen("output.txt","w",stdout);
    API api;
    api.createDownload(torrent_pack[0].first,torrent_pack[0].second);
    while (api.getInfo(torrent_pack[0].first).percent_download <= 75){
        //std::cout << api.getInfo(torrent_pack[2].first);
        //std::cout << "kek\n";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_GE(api.getInfo(torrent_pack[0].first).percent_download, 75);
}

TEST(CreateDownload, end_of_download){
    //freopen("output.txt","w",stdout);
    API api;
    api.createDownload(torrent_pack[0].first,torrent_pack[0].second);
    while (api.getInfo(torrent_pack[0].first).state != "finished"){
        std::cout << api.getInfo(torrent_pack[0].first).state <<"\n";
        //std::cout << api.getInfo(torrent_pack[2].first);
        //std::cout << "kek\n";
        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ASSERT_EQ(api.getInfo(torrent_pack[0].first).state,"finished");
}
