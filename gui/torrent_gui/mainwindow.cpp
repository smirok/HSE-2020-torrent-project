#include "mainwindow.h"
#include "ui_mainwindow.h"

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
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QThread>
#include <QFileDialog>


// =============================================WORKER=============================================

void Worker::start() {
    while (true) {
        emit update_screen();
        QThread::sleep(1);
    }
}


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
        auto *layout = new QHBoxLayout();
        auto *label = new QLabel(name);
        auto *progress = new QProgressBar();
        progress->setValue(0);
        layout->addWidget(label);
        layout->addWidget(progress);
        layout->setSizeConstraint( QLayout::SetFixedSize );
        w->setLayout(layout);

        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(w->sizeHint());

        cur_torrens_.emplace_back(i, name, location, true);
        ui_->list_cur_torrents->addItem(item);
        ui_->list_cur_torrents->setItemWidget(item, w);

        api_.createDownload(name.toStdString(), location.toStdString());
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
    QString buffer = json["count"].toString();            // TODO
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
    path_to_save_directory_ = "/home/andrey";
    ui_->setupUi(this);
    if (check_database("save.json")) {
        read_database("save.json");
    }
    ui_->list_cur_torrents->setContextMenuPolicy(Qt::CustomContextMenu);                                                           // TODO
    connect(ui_->list_cur_torrents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(show_context_menu(QPoint)));
    auto *thread = new QThread;
    auto *worker = new Worker;
    worker->moveToThread(thread);
    connect(worker, SIGNAL(update_screen()), this, SLOT(update_statistic()));
    connect(thread, SIGNAL(started()), worker, SLOT(start()));
    thread->start();
}


MainWindow::~MainWindow() {
    write_database("save.json");
    delete ui_;
}


// =============================================SLOTS=============================================

void MainWindow::on_action_open_torrent_triggered() {
    QString path_to_torrent = QFileDialog::getOpenFileName(this, "Select torrent file", "/home", "*.torrent");
    QFileInfo torrent_file(path_to_torrent);
    if (!torrent_file.exists()) {
        return;
    }
    if (!torrent_file.isReadable()) {
        QMessageBox::warning(this, "WARNING", "This is not a readable file");
        return;
    }

    int torrent_id = static_cast<int>(cur_torrens_.size());                                                      // TODO -- i need to create variable in the main class
    cur_torrens_.emplace_back(torrent_id, path_to_torrent, path_to_save_directory_, true);

    auto *w = new QWidget();
    auto *layout = new QHBoxLayout();
    auto *label = new QLabel(path_to_torrent);
    auto *progress = new QProgressBar();
    progress->setValue(0);
    layout->addWidget(label);
    layout->addWidget(progress);
    layout->setSizeConstraint( QLayout::SetFixedSize );
    w->setLayout(layout);
    auto *item = new QListWidgetItem;
    item->setSizeHint(w->sizeHint());
    ui_->list_cur_torrents->addItem(item);
    ui_->list_cur_torrents->setItemWidget(item, w);

    api_.createDownload(path_to_torrent.toStdString(),
                        path_to_save_directory_.toStdString());

}


void MainWindow::on_action_delete_torrent_triggered() {
    auto torrent = ui_->list_cur_torrents->currentItem();
    int row = ui_->list_cur_torrents->currentRow();
    if (torrent == nullptr) {
        QMessageBox::warning(this, "Error", "You haven't chosen a torrent");
        return;
    }
    std::string file_name = cur_torrens_[static_cast<size_t>(row)].name_.toStdString();
    ui_->list_cur_torrents->takeItem(row);
    cur_torrens_.erase(cur_torrens_.begin() + row); 
    api_.removeDownload(file_name, true);
}


void MainWindow::on_action_pause_torrent_triggered() {
    auto torrent = ui_->list_cur_torrents->currentItem();
    int row = ui_->list_cur_torrents->currentRow();
    if (torrent == nullptr) {
        QMessageBox::warning(this, "Error", "You haven't chosen a torrent");
        return;
    }
    std::string file_name = cur_torrens_[static_cast<size_t>(row)].name_.toStdString();
    cur_torrens_[static_cast<size_t>(row)].download_ = false;
    api_.pauseDownload(file_name);
}


void MainWindow::on_action_start_torrent_triggered() {
    auto torrent = ui_->list_cur_torrents->currentItem();
    int row = ui_->list_cur_torrents->currentRow();
    if (torrent == nullptr) {
        QMessageBox::warning(this, "Error", "You haven't chosen a torrent");
        return;
    }
    std::string file_name = cur_torrens_[static_cast<size_t>(row)].name_.toStdString();
    cur_torrens_[static_cast<size_t>(row)].download_ = true;
    api_.resumeDownload(file_name);
}


void MainWindow::update_statistic() {
    int row = ui_->list_cur_torrents->currentRow();
    if (row != -1) {
        if (cur_torrens_[static_cast<size_t>(row)].download_) {
            ui_->action_pause_torrent->setEnabled(true);
            ui_->action_start_torrent->setEnabled(false);
        } else {
            ui_->action_pause_torrent->setEnabled(false);
            ui_->action_start_torrent->setEnabled(true);
        }
    }

    for (size_t index = 0; index < cur_torrens_.size(); index++) {
        auto *w = new QWidget();
        auto *layout = new QHBoxLayout();
        auto *label = new QLabel(cur_torrens_[index].name_);
        auto *progress = new QProgressBar();
        auto *button = new QPushButton("Open in the folder");
        connect(button, &QPushButton::clicked, this, [=]() {
            QUrl url_location(cur_torrens_[index].locate_);                       // TODO
            QDesktopServices::openUrl(url_location);
        });
        std::string file_name = cur_torrens_[index].name_.toStdString();
        progress->setValue(static_cast<int>(api_.getInfo(file_name).percent_download));
        layout->addWidget(label);
        layout->addWidget(progress);

        if (progress->value() == 100) {
            layout->addWidget(button);
        }

        layout->setSizeConstraint( QLayout::SetFixedSize );
        w->setLayout(layout);

        auto *item = ui_->list_cur_torrents->item(static_cast<int>(index));
        item->setSizeHint(w->sizeHint());
        ui_->list_cur_torrents->setItemWidget(item, w);
    }
}


void MainWindow::show_context_menu(const QPoint &pos) {
    QPoint point = ui_->list_cur_torrents->mapToGlobal(pos);
    QMenu menu;
    menu.addAction("Delete",  this, SLOT(on_action_delete_torrent_triggered()));
    menu.addAction("Information",  this, SLOT(on_action_delete_torrent_triggered()));
    menu.exec(point);
}
