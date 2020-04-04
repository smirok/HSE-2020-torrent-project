#include <iostream>
#include "API.h"

int main() {
    API api; // тут запускается сессия
    std::string directory;
    directory = "/home/ilya/lildojd";
    std::vector<std::string> v;
    v.emplace_back("udp://192.168.0.107:8001");
    api.makeTorrent(directory,v);
    /*std::cout << "Место установки: \n";
    std::cin >> directory;
    std::cout << "Путь до файла .torrent : \n";
    std::cin >> file_name;
    std::cout << "Место установки 2 : \n";
    std::cin >> directory2;
    std::cout << "Путь до файла .torrent 2: \n";
    std::cin >> file_name2;
    api.createDownload(file_name, directory);
    api.createDownload(file_name2,directory2);
    for (int i = 0; i < 1e8; i++) {
        std::cout << api.getInfo(file_name);
        std::cout << api.getInfo(file_name2);
    }*/
    return 0;
}

// symbolic link на торрент

// посмотреть библиотеку в ppa

// thread pool для парсинга запросов - vector<thread>

// rutracker