#include "torrent_client.h"
#include "API.h"
#include <QThread>
#include <iostream>


// ============================================METHODS============================================

TorrentClient::TorrentClient(QString file_name, int id) : file_name_(file_name), id_(id) {}


// =============================================SLOTS=============================================

void TorrentClient::start_load() {
    API api;
    api.enter_dir(".");
    api.enter_file(file_name_.toStdString());
    api.start_download();
    for (int i = 0; i <= 100; i += 10) {
        emit send_statistic(i, id_);
        QThread::sleep(3);
    }
}

