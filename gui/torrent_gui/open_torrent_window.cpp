#include "open_torrent_window.h"
#include "ui_open_torrent_window.h"


#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>


OpenTorrentWindow::OpenTorrentWindow(QWidget *parent) : QDialog(parent), ui(new Ui::OpenTorrentWindow) {
    ui->setupUi(this);
}


OpenTorrentWindow::~OpenTorrentWindow() {
    delete ui;
}


void OpenTorrentWindow::on_select_torrent_button_clicked() {
    QString name = QFileDialog::getOpenFileName(this, "Select torrent file", "/home", "*.torrent");
    ui->torrent_line->setText(name);
}


void OpenTorrentWindow::on_select_directory_button_clicked() {
    QString name = QFileDialog::getExistingDirectory(this, "Select directory", "/home");
    ui->directory_line->setText(name);
}


void OpenTorrentWindow::on_add_torrent_button_clicked() {
    std::string path_to_torrent = (ui->torrent_line->text()).toStdString();
    std::string path_to_directory = (ui->directory_line->text()).toStdString();

    if (!boost::filesystem::exists(path_to_torrent)) {
        QMessageBox::warning(this, "Wrong path", "Torrent file doesn't exist");
        return;
    }
    if (!boost::filesystem::exists(path_to_directory)) {
        QMessageBox::warning(this, "Wrong path", "Directory doesn't exist");
        return;
    }

    close();
}
