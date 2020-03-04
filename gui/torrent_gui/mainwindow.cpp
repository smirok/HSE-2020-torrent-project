#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "open_torrent_window.h"


#include <iostream>
#include <vector>
#include <QMessageBox>


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

    cur_torrens_.push_back(Torrent(window.path_to_torrent_, window.path_to_save_directory_));
    QStringList buffer(window.path_to_torrent_);
    ui_->list_cur_torrents->addItems(buffer);

}

void MainWindow::on_action_delete_torrent_triggered() {
    auto torrent = ui_->list_cur_torrents->currentItem();
    int row = ui_->list_cur_torrents->currentRow();
    if (torrent == nullptr) {
        QMessageBox::warning(this, "Error", "You haven't chosen a torrent");
        return;
    }
    ui_->list_cur_torrents->takeItem(row);
    cur_torrens_.erase(cur_torrens_.begin() + row);
}
