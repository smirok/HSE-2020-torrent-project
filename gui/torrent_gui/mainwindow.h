#pragma once


#include "API.h"

#include <QMainWindow>
#include <QThread>
#include <QListWidgetItem>

#include <vector>


class Worker : public QObject {
    Q_OBJECT

public slots:
    void start();

signals:
    void update_screen();

};


struct Torrent {
    Torrent(int id, QString name, QString locate, bool dowload) {
        id_ = id;
        name_= name;
        locate_ = locate;
        download_ = dowload;
    }
    int id_;
    QString name_;
    QString locate_;
    bool download_;
};


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_open_torrent_triggered();
    void on_action_delete_torrent_triggered();
    void on_action_pause_torrent_triggered();
    void on_action_start_torrent_triggered();

    void update_statistic();
    void show_context_menu(const QPoint &point);

    void on_list_cur_torrents_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MainWindow *ui_;
    std::vector<Torrent> cur_torrens_;
    API api_;

    void read_database(QString file_name);
    void write_database(QString file_name);
    bool check_database(QString file_name);

};

