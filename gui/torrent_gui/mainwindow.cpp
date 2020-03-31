#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "open_torrent_window.h"

#include <iostream>
#include <vector>
#include <cassert>

#include <QWidget>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QProgressBar>


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
        QFileInfo torrent_file(name);
        if (!torrent_file.isFile() || torrent_file.suffix().toStdString() != "torrent") {
            std::cout << "WARNING: This is not a torrent file -- " << name.toStdString() << std::endl;
            continue;
        }
        if (!torrent_file.isReadable()) {
            std::cout << "WARNING: This is not a readable file -- " << name.toStdString() << std::endl;
            continue;
        }

        QString location = json[id].toObject()["location"].toString();
        QFileInfo save_directory(location);
        if (!save_directory.isDir()) {
            std::cout << "WARNING: This is not a directory -- " << location.toStdString() << std::endl;
            continue;
        }
        if (!save_directory.isWritable()) {
            std::cout << "WARNING: This is not a writable directory -- " << location.toStdString() << std::endl;
            continue;
        }

        auto *w = new QWidget();
        auto *vbl = new QHBoxLayout();
        auto *label = new QLabel(name);
        auto *progress = new QProgressBar();
        progress->setValue(100);
        vbl->addWidget(label);
        vbl->addWidget(progress);
        vbl->setSizeConstraint( QLayout::SetFixedSize );
        w->setLayout(vbl);

        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(w->sizeHint());

        cur_torrens_.emplace_back(Torrent(i, name, location));
        ui_->list_cur_torrents->addItem(item);
        ui_->list_cur_torrents->setItemWidget(item, w);
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
        if (buffer["name"].isNull()) {
           return false;
        }
        if (buffer["location"].isNull()) { 
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
    ui_->list_cur_torrents->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_->list_cur_torrents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(show_context_menu(QPoint)));
}


MainWindow::~MainWindow() {
    write_database("save.json");
    delete ui_;
}


int MainWindow::find_index(int torrent_id) {
    int index = -1;
    for (size_t i = 0; i < cur_torrens_.size(); i++) {
        if (cur_torrens_[i].id_ == torrent_id) {
            index = static_cast<int>(i);
        }
    }
    return index;
}


// =============================================SLOTS=============================================

void MainWindow::on_action_open_torrent_triggered() {
    OpenTorrentWindow window;
    window.setModal(true);
    window.exec();

    if (!window.data_is_read_) {
        return;
    }

    int torrent_id = static_cast<int>(cur_torrens_.size());
    cur_torrens_.emplace_back(Torrent(torrent_id, window.path_to_torrent_, window.path_to_save_directory_));

    auto *w = new QWidget();
    auto *vbl = new QHBoxLayout();
    auto *label = new QLabel(window.path_to_torrent_);
    auto *progress = new QProgressBar();
    progress->setValue(0);
    vbl->addWidget(label);
    vbl->addWidget(progress);
    vbl->setSizeConstraint( QLayout::SetFixedSize );
    w->setLayout(vbl);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(w->sizeHint());
    ui_->list_cur_torrents->addItem(item);
    ui_->list_cur_torrents->setItemWidget(item, w);

    QThread *thread = new QThread;
    TorrentClient *torrent_client = new TorrentClient(window.path_to_torrent_, torrent_id);
    torrent_client->moveToThread(thread);
    connect(torrent_client, SIGNAL(send_statistic(int, int)), this, SLOT(update_statistic(int, int)));
    connect(thread, SIGNAL(started()), torrent_client, SLOT(start_load()));
    thread->start();
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


void MainWindow::update_statistic(int percent, int torrent_id) {
    int index = find_index(torrent_id);
    if (index == -1) {
        return;
    }
    assert(index >= 0);

    auto *w = new QWidget();
    auto *vbl = new QHBoxLayout();
    auto *label = new QLabel(cur_torrens_[static_cast<size_t>(index)].name_);
    auto *progress = new QProgressBar();
    progress->setValue(percent);
    vbl->addWidget(label);
    vbl->addWidget(progress);
    vbl->setSizeConstraint( QLayout::SetFixedSize );
    w->setLayout(vbl);

    auto *item = ui_->list_cur_torrents->item(index);
    item->setSizeHint(w->sizeHint());
    ui_->list_cur_torrents->setItemWidget(item, w);
}


void MainWindow::show_context_menu(const QPoint &pos) {
    QPoint point = ui_->list_cur_torrents->mapToGlobal(pos);
    QMenu menu;
    menu.addAction("Delete",  this, SLOT(on_action_delete_torrent_triggered()));
    menu.exec(point);
}
