#pragma once


#include <QDialog>


namespace Ui {
class OpenTorrentWindow;
}


class OpenTorrentWindow : public QDialog {

    Q_OBJECT

public:
    explicit OpenTorrentWindow(QWidget *parent = nullptr);
    ~OpenTorrentWindow();

private slots:
    void on_select_torrent_button_clicked();

    void on_select_directory_button_clicked();

    void on_add_torrent_button_clicked();

private:
    Ui::OpenTorrentWindow *ui;

};
