#include <iostream>
#include "API.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    API api;
    std::string directory,file_name;
    std::cout << "Место установки: \n";
    std::cin >> directory;
    std::cout << "Путь до файла .torrent : \n";
    api.enter_dir(directory);// to do
    api.enter_file(file_name);
    api.start_download();
    return 0;
}

// symbolic link на торрент

// посмотреть библиотеку в ppa

// версия библиотек на ubuntu

// напиcать в readme как скачивать, какую версию
