#include "torrent_client.h"
// #include "API.h"


#include <QThread>
#include <iostream>


// ============================================METHODS============================================

TorrentClient::TorrentClient(QString file_name, int id) : file_name_(file_name), id_(id) {}


// =============================================SLOTS=============================================

void TorrentClient::start_load() {
//    std::string buffer = file_name_.toStdString();
//    api_.enter_dir(".");
//    api_.enter_file(buffer);
//    api_.start_download();
//    std::cout << "hello fucking world" << std::endl;
    for (int i = 0; i <= 100; i++) {
        emit send_statistic(i, id_);
        QThread::sleep(1);
    }
}

