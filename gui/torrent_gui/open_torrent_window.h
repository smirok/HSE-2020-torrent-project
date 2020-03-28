#pragma once


#include <QDialog>
#include <iostream>


namespace Ui {
class OpenTorrentWindow;
}


class OpenTorrentWindow : public QDialog {

    Q_OBJECT

public:
    explicit OpenTorrentWindow(QWidget *parent = nullptr);
    ~OpenTorrentWindow();

    QString path_to_torrent_ = "";                                  // TODO -- maybe these variables are not public
    QString path_to_save_directory_ = "";
    bool data_is_read_ = false;

private slots:
    void on_select_torrent_button_clicked();
    void on_select_directory_button_clicked();
    void on_add_torrent_button_clicked();

private:
    Ui::OpenTorrentWindow *ui_;

};
