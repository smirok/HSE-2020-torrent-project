#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "open_torrent_window.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::on_add_torrent_button_clicked() {

    OpenTorrentWindow window;
    window.setModal(true);
    window.exec();

}
