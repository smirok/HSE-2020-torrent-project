#include <iostream>
#include "API.h"

int main() {
    API api; // тут запускается сессия
    std::string directory, directory2, file_name, file_name2;
    std::cout << "Место установки: \n";
    std::cin >> directory;
    std::cout << "Путь до файла .torrent : \n";
    std::cin >> file_name;
    std::cout << "Место установки 2 : \n";
    std::cin >> directory2;
    std::cout << "Путь до файла .torrent 2: \n";
    std::cin >> file_name2;
    api.createDownload(file_name, directory);
    std::cout << api.view.session_handles.size() << std::endl;
    api.createDownload(file_name2,directory2);
    std::cout << api.view.session_handles.size() << std::endl;
    //api.pauseDownload(file_name);
    for (int i = 0; i < 1e8; i++) {
        std::cout << api.getInfo(file_name);
        std::cout << api.getInfo(file_name2);
    }
    api.removeDownload(file_name);
    api.removeDownload(file_name2);
    std::cout << api.view.session_handles.size() << std::endl;
    return 0;
}

// symbolic link на торрент

// посмотреть библиотеку в ppa

// thread pool для парсинга запросов - vector<thread>

// rutracker