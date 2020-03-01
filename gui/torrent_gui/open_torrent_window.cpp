#include "open_torrent_window.h"
#include "ui_open_torrent_window.h"

OpenTorrentWindow::OpenTorrentWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenTorrentWindow)
{
    ui->setupUi(this);
}

OpenTorrentWindow::~OpenTorrentWindow()
{
    delete ui;
}
