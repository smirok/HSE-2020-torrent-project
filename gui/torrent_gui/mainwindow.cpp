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


// =============================================JSON=============================================


void MainWindow::read_database(QString file_name) {
    QFile file(file_name);
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject json(doc.object());

    int count_torrent = json["count"].toString().toInt();

    for (int i = 0; i < count_torrent; i++) {
        QString id = QString::number(i);
        QString name = json[id].toObject()["name"].toString();
        QString location = json[id].toObject()["location"].toString();
        cur_torrens_.emplace_back(Torrent(name, location));
        ui_->list_cur_torrents->addItem(name);
    }
}


void MainWindow::write_database(QString file_name) {
    QJsonObject database;

    database.insert("count", QString::number(cur_torrens_.size()));
    for (size_t i = 0; i < cur_torrens_.size(); i++) {
        QJsonObject torrent;
        torrent.insert("name", cur_torrens_[i].name_);
        torrent.insert("location", cur_torrens_[i].locate_);
        database.insert(QString::number(i), torrent);
    }

    QJsonDocument doc(database);
    QFile file(file_name);
    file.open(QIODevice::WriteOnly);
    file.write(doc.toJson());
}


bool MainWindow::check_database(QString file_name) {
    QFile file(file_name);
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject json(doc.object());

    // checking the count-parameter
    if (json["count"].isNull()) {
        return false;
   }
    QString buffer = json["count"].toString();
    for (auto e : buffer) {
        if (e < '0' || '9' < e) {
            return false;
        }
    }
    int count_torrent = buffer.toInt();
    for (int i = 0; i < count_torrent; i++) {
        QString id = QString::number(i);
        if (json[id].isNull()) {
            return false;
        }
    }

    // checking the torrent-parameters
    for (int i = 0; i < count_torrent; i++) {
        QString id = QString::number(i);
        auto buffer = json[id].toObject();
        if (buffer["name"].isNull()) {                         // Add to check the correctness for path to torrent file
           return false;
        }
        if (buffer["location"].isNull()) {                     // Add to check the correctness for path to save directory
           return false;
        }
    }

    return true;
}


// ============================================METHODS============================================


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);
    if (check_database("save.json")) {
        read_database("save.json");
    }
}


MainWindow::~MainWindow() {
    write_database("save.json");
    delete ui_;
}


// =============================================SLOTS=============================================


void MainWindow::on_action_open_torrent_triggered() {
    OpenTorrentWindow window;
    window.setModal(true);
    window.exec();

    if (!window.data_is_read_) {
        return;
    }

    cur_torrens_.emplace_back(Torrent(window.path_to_torrent_, window.path_to_save_directory_));
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
