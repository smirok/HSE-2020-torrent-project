#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "open_torrent_window.h"


#include <iostream>
#include <vector>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);
}


MainWindow::~MainWindow() {
    delete ui_;
}


void MainWindow::on_action_open_torrent_triggered() {
    OpenTorrentWindow window;
    window.setModal(true);
    window.exec();

    if (!window.data_is_read_) return;

    cur_torrens_.push_back(Torrent(window.path_to_torrent_));
    QStringList buffer(window.path_to_torrent_);
    ui_->list_cur_torrents->addItems(buffer);

}
