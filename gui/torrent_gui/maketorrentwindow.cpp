#include "maketorrentwindow.h"
#include "ui_maketorrentwindow.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QStringList>

#include <iostream>


MakeTorrentWindow::MakeTorrentWindow(QWidget *parent) : QDialog(parent), ui_(new Ui::MakeTorrentWindow) {
    ui_->setupUi(this);
    ui_->radio_button_folder->setChecked(false);
    ui_->radio_button_file->setChecked(true);
    ui_->push_button_folder->setEnabled(false);
    ui_->push_button_file->setEnabled(true);

    this->setFixedSize(this->width(), this->height());
}


MakeTorrentWindow::~MakeTorrentWindow() {
    delete ui_;
}


void MakeTorrentWindow::on_radio_button_folder_clicked() {
    ui_->radio_button_folder->setChecked(true);
    ui_->radio_button_file->setChecked(false);
    ui_->push_button_folder->setEnabled(true);
    ui_->push_button_file->setEnabled(false);
}


void MakeTorrentWindow::on_radio_button_file_clicked() {
    ui_->radio_button_folder->setChecked(false);
    ui_->radio_button_file->setChecked(true);
    ui_->push_button_folder->setEnabled(false);
    ui_->push_button_file->setEnabled(true);
}


void MakeTorrentWindow::on_push_button_save_location_clicked() {
    path_to_save_directory_ = QFileDialog::getExistingDirectory(this, "Select the save directory", "/home");

    QFileInfo save_directory(path_to_save_directory_);
    if (!save_directory.isWritable()) {
        QMessageBox::warning(this, "WARNING", "This is not a writable directory");
        return;
    }

    ui_->text_browser_save_location->setText(path_to_save_directory_);
}


void MakeTorrentWindow::on_push_button_folder_clicked() {
    path_to_source_ = QFileDialog::getExistingDirectory(this, "Select the source folder", "/home");

    QFileInfo source_folder(path_to_source_);
    if (!source_folder.isReadable()) {
        QMessageBox::warning(this, "WARNING", "This is not a readable directory");
        return;
    }

    ui_->text_browser_source->setText(path_to_source_);
}


void MakeTorrentWindow::on_push_button_file_clicked() {
    path_to_source_ = QFileDialog::getOpenFileName(this, "Select the source file", "/home");

    QFileInfo source_file(path_to_source_);
    if (!source_file.isReadable()) {
        QMessageBox::warning(this, "WARNING", "This is not a readable file");
        return;
    }

    ui_->text_browser_source->setText(path_to_source_);
}


void MakeTorrentWindow::on_push_button_new_clicked() {
    trackers_.clear();

    QVector<QString> trackers = ui_->text_edit_trackers->toPlainText().split('\n').toVector();
    for (auto tracker : trackers) {
        if (!tracker.isEmpty()) {                           // TO DO -- Here I need to check the correctness of this tracker.
            trackers_.push_back(tracker);
        }
    }

    name_ = ui_->line_edit_name->text();

    if (path_to_save_directory_.isEmpty()) {
        QMessageBox::warning(this, "WARNING", "Choose the save directory");
        return;
    }
    if (path_to_source_.isEmpty()) {
        QMessageBox::warning(this, "WARNING", "Choose the source file/folder");
        return;
    }
    if (trackers_.isEmpty()) {
        QMessageBox::warning(this, "WARNING", "Enter at least one tracker");
        return;
    }
    if (name_.isEmpty()) {
        QMessageBox::warning(this, "WARNING", "Enter the name");
        return;
    }

    good_ = true;
    close();
}


void MakeTorrentWindow::on_push_button_close_clicked() {
    close();
}


std::string MakeTorrentWindow::get_save_directory() {
    return path_to_save_directory_.toStdString();
}


std::string MakeTorrentWindow::get_source() {
    return path_to_source_.toStdString();
}


std::vector<std::string> MakeTorrentWindow::get_trackers() {
    std::vector<std::string> result;
    for (auto tracker : trackers_) {
        result.push_back(tracker.toStdString());
    }
    return result;
}


std::string MakeTorrentWindow::get_name() {
    return name_.toStdString();
}


bool MakeTorrentWindow::is_good() {
    return good_;
}
