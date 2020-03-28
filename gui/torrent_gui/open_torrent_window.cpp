#include "open_torrent_window.h"
#include "ui_open_torrent_window.h"

#include <iostream>

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>


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

    QFileInfo torrent_file(path_to_torrent_);
    if (!torrent_file.isFile() || torrent_file.suffix().toStdString() != "torrent") {
        QMessageBox::warning(this, "WARNING", "This is not a torrent file");
        return;
    }
    if (!torrent_file.isReadable()) {
        QMessageBox::warning(this, "WARNING", "This is not a readable file");
    }

    QFileInfo save_directory(path_to_save_directory_);
    if (!save_directory.isDir()) {
        QMessageBox::warning(this, "WARNING", "This is not a directory");
        return;
    }
    if (!save_directory.isWritable()) {
        QMessageBox::warning(this, "WARNING", "This is not a writable directory");
        return;
    }

    data_is_read_ = true;

    close();
}
