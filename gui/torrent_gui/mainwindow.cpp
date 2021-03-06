#include "mainwindow.h"
#include "maketorrentwindow.h"
#include "addtorrentwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <ctime>

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
#include <QDir>
#include <QDialog>
#include <QPalette>
#include <QColor>


// =============================================WORKER=============================================

void Worker::start() {
    while (program_is_running) {
        emit update_screen();
        QThread::msleep(400);
    }
}


// =============================================JSON=============================================

void MainWindow::read_database(QString database) {
    QFile file(database);
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject json(doc.object());

    int count_torrent = json["count"].toString().toInt();
    for (int i = 0; i < count_torrent; i++) {
        QString id = QString::number(i);

        QString filename = json[id].toObject()["name"].toString();
        QFileInfo torrent_file(filename);
        if (!torrent_file.isFile() || torrent_file.suffix().toStdString() != "torrent") {
            std::cerr << "WARNING: This is not a torrent file -- " << filename.toStdString() << std::endl;
            continue;
        }
        if (!torrent_file.isReadable()) {
            std::cerr << "WARNING: This is not a readable file -- " << filename.toStdString() << std::endl;
            continue;
        }

        QString location = json[id].toObject()["location"].toString();
        QFileInfo save_directory(location);
        if (!save_directory.isDir()) {
            std::cerr << "WARNING: This is not a directory -- " << location.toStdString() << std::endl;
            continue;
        }
        if (!save_directory.isWritable()) {
            std::cerr << "WARNING: This is not a writable directory -- " << location.toStdString() << std::endl;
            continue;
        }

        QString marks = json[id].toObject()["marks"].toString();

        auto *widget = new QWidget(this);
        auto *layout = new QHBoxLayout(widget);
        auto *label = new QLabel(filename, widget);

        auto *progress = new QProgressBar(this);
        QPalette palette = progress->palette();
        palette.setColor(QPalette::Highlight, QColor(0, 191, 255));
        palette.setColor(QPalette::HighlightedText, Qt::black);
        progress->setPalette(palette);
        progress->setValue(0);

        layout->addWidget(label);
        layout->addWidget(progress);
        layout->setSizeConstraint(QLayout::SetFixedSize);
        widget->setLayout(layout);

        QListWidgetItem *item = new QListWidgetItem(ui_->list_cur_torrents);
        item->setSizeHint(widget->sizeHint());

        cur_torrens_.push_back({filename, location, marks, progress});
        ui_->list_cur_torrents->addItem(item);
        ui_->list_cur_torrents->setItemWidget(item, widget);

        api_.prepareDownload(filename.toStdString(), location.toStdString());
        api_.pickDownloadFiles();
        for (int i = 0; i < marks.size(); i++) {
            if (marks[i] == "1") {
                api_.picker.setMark(i, true);
            }
        }
        api_.createDownload(filename.toStdString());
    }
}


void MainWindow::write_database(QString file_name) {
    QJsonObject database;

    database.insert("count", QString::number(cur_torrens_.size()));
    for (size_t i = 0; i < cur_torrens_.size(); i++) {
        QJsonObject torrent;
        torrent.insert("name", cur_torrens_[i].name_);
        torrent.insert("location", cur_torrens_[i].locate_);
        torrent.insert("marks", cur_torrens_[i].marks_);
        database.insert(QString::number(i), torrent);
    }

    QJsonDocument doc(database);
    QFile file(file_name);
    file.open(QIODevice::WriteOnly);
    file.write(doc.toJson());
}


bool MainWindow::check_database(QString file_name) {
    QFileInfo database(file_name);
    if (!database.exists()) {
        return false;
    }

    QFile file(file_name);
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc(QJsonDocument::fromJson(file.readAll()));
    QJsonObject json(doc.object());

    // checking the count-parameter
    if (json["count"].isNull()) {
        return false;
    }
    QString buffer = json["count"].toString();
    for (auto symbol : buffer) {
        if (symbol < '0' || '9' < symbol) {
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
        if (buffer["marks"].isNull()) {
           return false;
        }
    }

    return true;
}


// ============================================METHODS============================================

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);

    ui_->action_pause_torrent->setEnabled(false);
    ui_->action_start_torrent->setEnabled(false);
    ui_->action_delete_torrent->setEnabled(false);

    if (check_database(QDir::homePath() + "/.config/TorrentX.json")) {
        read_database(QDir::homePath() + "/.config/TorrentX.json");
    }

    ui_->list_cur_torrents->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_->list_cur_torrents, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(show_context_menu(QPoint)));

    thread_ = new QThread();
    auto *worker = new Worker();
    worker->moveToThread(thread_);
    connect(worker, SIGNAL(update_screen()), this, SLOT(update_statistic()));
    connect(thread_, SIGNAL(started()), worker, SLOT(start()));
    thread_->start();
}


MainWindow::~MainWindow() {
    program_is_running = false;
    thread_->quit();
    thread_->wait();

    write_database(QDir::homePath() + "/.config/TorrentX.json");

    delete ui_;
}


// =============================================SLOTS=============================================

void MainWindow::on_action_open_torrent_triggered() {
    QString path_to_torrent = QFileDialog::getOpenFileName(this, "Select the torrent file", "/home", "*.torrent");
    if (path_to_torrent.isEmpty()) {
        return;
    }
    QFileInfo torrent_file(path_to_torrent);
    if (!torrent_file.isReadable()) {
        QMessageBox::warning(this, "WARNING", "This is not a readable file");
        return;
    }

    QString path_to_save_directory = QFileDialog::getExistingDirectory(this, "Select the save directory", "/home");
    if (path_to_save_directory.isEmpty()) {
        return;
    }
    QFileInfo save_directory(path_to_save_directory);
    if (!save_directory.isWritable()) {
        QMessageBox::warning(this, "WARNING", "This is not a writable directory");
        return;
    }

    AddTorrentWindow window(nullptr, api_, path_to_torrent, path_to_save_directory);
    window.setModal(true);
    window.exec();

    if (!window.is_good()) {
        return;
    }

    auto *widget = new QWidget(this);
    auto *layout = new QHBoxLayout(widget);
    auto *label = new QLabel(path_to_torrent, widget);

    auto *progress = new QProgressBar(this);
    QPalette palette = progress->palette();
    palette.setColor(QPalette::Highlight, QColor(0, 191, 255));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    progress->setPalette(palette);
    progress->setValue(0);

    layout->addWidget(label);
    layout->addWidget(progress);
    widget->setLayout(layout);



    auto *item = new QListWidgetItem(ui_->list_cur_torrents);
    item->setSizeHint(widget->sizeHint());

    QString marks;
    for (auto element : api_.picker.getMarks()) {
        if (element) {
            marks += "1";
        } else {
            marks += "0";
        }
    }

    cur_torrens_.push_back({path_to_torrent, path_to_save_directory, marks, progress});
    ui_->list_cur_torrents->addItem(item);
    ui_->list_cur_torrents->setItemWidget(item, widget);

    api_.createDownload(path_to_torrent.toStdString());
}


void MainWindow::on_action_create_torrent_triggered() {
    MakeTorrentWindow window;
    window.setModal(true);
    window.exec();

    if (!window.is_good()) {
        return;
    }

    std::string source = window.get_source();
    std::vector<std::string> trackers = window.get_trackers();
    std::string save_directory = window.get_save_directory();
    std::string name = window.get_name();

    api_.makeTorrent(source, trackers, save_directory, name);
}


void MainWindow::on_action_delete_torrent_triggered() {
    QMessageBox message_box(this);
    message_box.setWindowTitle("Delete");
    message_box.setIcon(QMessageBox::Question);
    message_box.setText("Do you want to detele the received files?");
    message_box.addButton(QMessageBox::Cancel);
    message_box.addButton(QMessageBox::No);
    message_box.addButton(QMessageBox::Yes);
    auto reply = message_box.exec();

    if (reply == QMessageBox::Cancel) {
        return;
    }

    int row = ui_->list_cur_torrents->currentRow();
    std::string filename = cur_torrens_[static_cast<size_t>(row)].name_.toStdString();

    ui_->list_cur_torrents->takeItem(row);
    cur_torrens_.erase(cur_torrens_.begin() + row);

    api_.removeDownload(filename, reply == QMessageBox::Yes);
}


void MainWindow::on_action_pause_torrent_triggered() {
    size_t row = static_cast<size_t>(ui_->list_cur_torrents->currentRow());
    std::string file_name = cur_torrens_[row].name_.toStdString();

    cur_torrens_[row].download_ = false;
    api_.pauseDownload(file_name);

    QPalette palette = cur_torrens_[row].progress_bar_->palette();
    palette.setColor(QPalette::Highlight, Qt::gray);
    cur_torrens_[row].progress_bar_->setPalette(palette);

    ui_->action_pause_torrent->setEnabled(false);
    ui_->action_start_torrent->setEnabled(true);
}


void MainWindow::on_action_start_torrent_triggered() {
    int row = ui_->list_cur_torrents->currentRow();
    std::string file_name = cur_torrens_[static_cast<size_t>(row)].name_.toStdString();

    cur_torrens_[static_cast<size_t>(row)].download_ = true;
    api_.resumeDownload(file_name);

    QPalette palette = cur_torrens_[row].progress_bar_->palette();
    palette.setColor(QPalette::Highlight, QColor(0, 191, 255));
    cur_torrens_[row].progress_bar_->setPalette(palette);

    ui_->action_pause_torrent->setEnabled(true);
    ui_->action_start_torrent->setEnabled(false);
}


void MainWindow::open_in_folder() {
    int row = ui_->list_cur_torrents->currentRow();
    QString location = cur_torrens_[static_cast<size_t>(row)].locate_;
    QUrl url_location(location);
    QDesktopServices::openUrl(url_location);
}


void MainWindow::update_statistic() {
    for (size_t index = 0; index < cur_torrens_.size(); index++) {
        Torrent &torrent = cur_torrens_[index];

        auto *widget = new QWidget(this);
        auto *layout = new QHBoxLayout(widget);
        auto *label = new QLabel(torrent.name_, widget);

        std::string file_name = torrent.name_.toStdString();

        torrent.progress_bar_->setValue(static_cast<int>(api_.getInfo(file_name).percent_download_));
        layout->addWidget(label);
        layout->addWidget(torrent.progress_bar_);

        if (api_.getInfo(file_name).state_ == "downloading") {

            std::string memory = api_.getInfo(file_name).progress_info_;
            memory += "   | ";
            auto *memory_label = new QLabel(QString::fromStdString(memory), widget);
            layout->addWidget(memory_label);

            std::string time = api_.getInfo(file_name).remain_time_;
            auto *time_label = new QLabel(widget);

            if (torrent.download_) {
                time_label->setText(QString::fromStdString(time));
            } else {
                time_label->setText("PAUSE");
            }

            layout->addWidget(time_label);

        }

        //

        if (torrent.progress_bar_->value() == 100) {
            torrent.finished_ = true;
            QPalette palette = torrent.progress_bar_->palette();
            palette.setColor(QPalette::Highlight, Qt::green);
            torrent.progress_bar_->setPalette(palette);
        }

        layout->setSizeConstraint(QLayout::SetFixedSize);
        widget->setLayout(layout);

        auto *item = ui_->list_cur_torrents->item(static_cast<int>(index));
        item->setSizeHint(widget->sizeHint());
        ui_->list_cur_torrents->setItemWidget(item, widget);
    }

    int row = ui_->list_cur_torrents->currentRow();
    if (row != -1 && cur_torrens_[static_cast<size_t>(row)].finished_) {
        ui_->action_pause_torrent->setEnabled(false);
        ui_->action_start_torrent->setEnabled(false);
        ui_->action_delete_torrent->setEnabled(true);
    }
}


void MainWindow::show_context_menu(const QPoint &pos) {
    QPoint point = ui_->list_cur_torrents->mapToGlobal(pos);
    QMenu menu;
    menu.addAction("Delete",  this, SLOT(on_action_delete_torrent_triggered()));
    menu.addAction("Open Folder",  this, SLOT(open_in_folder()));

    if (ui_->list_cur_torrents->currentRow() == -1) {
        for (auto *element : menu.actions()) {
            element->setEnabled(false);
        }
    }

    menu.exec(point);
}


void MainWindow::on_list_cur_torrents_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    (void) current;
    (void) previous;

    int row = ui_->list_cur_torrents->currentRow();
    if (row != -1) {
        if (cur_torrens_[static_cast<size_t>(row)].finished_) {
            ui_->action_pause_torrent->setEnabled(false);
            ui_->action_start_torrent->setEnabled(false);
            ui_->action_delete_torrent->setEnabled(true);
        } else if (cur_torrens_[static_cast<size_t>(row)].download_) {
            ui_->action_pause_torrent->setEnabled(true);
            ui_->action_start_torrent->setEnabled(false);
            ui_->action_delete_torrent->setEnabled(true);
        } else {
            ui_->action_pause_torrent->setEnabled(false);
            ui_->action_start_torrent->setEnabled(true);
            ui_->action_delete_torrent->setEnabled(true);
        }
    } else {
        ui_->action_pause_torrent->setEnabled(false);
        ui_->action_start_torrent->setEnabled(false);
        ui_->action_delete_torrent->setEnabled(false);
    }
}
