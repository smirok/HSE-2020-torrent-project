#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "open_torrent_window.h"


#include <iostream>
#include <vector>
#include <QMessageBox>
#include <QJsonDocument>
#include <QDebug>
#include <QFile>
#include <QJsonObject>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {

    ui_->setupUi(this);

    QFile file("save.json");
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject json(doc.object());
    file.close();

    int count_torrent = json["count"].toString().toInt();

    for (int i = 0; i < count_torrent; i++) {
        QString id = QString::number(i);
        QString name = json[id].toObject()["name"].toString();
        QString location = json[id].toObject()["location"].toString();
        cur_torrens_.push_back(Torrent(name, location));
        ui_->list_cur_torrents->addItem(name);
    }

}


MainWindow::~MainWindow() {
    QJsonObject data_base;
    data_base.insert("count", QString::number(cur_torrens_.size()));
    for (size_t i = 0; i < cur_torrens_.size(); i++) {
        QJsonObject torrent;
        torrent.insert("name", cur_torrens_[i].name_);
        torrent.insert("location", cur_torrens_[i].name_locate_);
        data_base.insert(QString::number(i), torrent);
    }
    QJsonDocument doc(data_base);
    QFile file("save.json");
    file.open(QIODevice::WriteOnly);
    file.write(doc.toJson());
    file.close();
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
