#include "open_torrent_window.h"
#include "ui_open_torrent_window.h"


#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>


OpenTorrentWindow::OpenTorrentWindow(QWidget *parent) : QDialog(parent), ui_(new Ui::OpenTorrentWindow) {
    ui_->setupUi(this);
}


OpenTorrentWindow::~OpenTorrentWindow() {
    delete ui_;
}


void OpenTorrentWindow::on_select_torrent_button_clicked() {
    QString name = QFileDialog::getOpenFileName(this, "Select torrent file", "/home", "*.torrent");
    ui_->torrent_line->setText(name);
}


void OpenTorrentWindow::on_select_directory_button_clicked() {
    QString name = QFileDialog::getExistingDirectory(this, "Select directory", "/home");
    ui_->directory_line->setText(name);
}


void OpenTorrentWindow::on_add_torrent_button_clicked() {
    path_to_torrent_ = ui_->torrent_line->text();
    path_to_save_directory_ = ui_->directory_line->text();

    if (!boost::filesystem::exists(path_to_torrent_.toStdString())) {
        QMessageBox::warning(this, "Wrong path", "Torrent file doesn't exist");
        return;
    }
    if (!boost::filesystem::exists(path_to_save_directory_.toStdWString())) {
        QMessageBox::warning(this, "Wrong path", "Save directory doesn't exist");
        return;
    }

    data_is_read_ = true;

    close();
}
