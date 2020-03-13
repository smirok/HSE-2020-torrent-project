#pragma once


#include "torrent_client.h"

#include <QMainWindow>
#include <QThread>
#include <vector>


struct Torrent {
    Torrent(int id, QString name, QString locate) : id_(id), name_(name), locate_(locate) {}
    int id_;
    QString name_;
    QString locate_;
    // ...
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
    void update_statistic(int percent, int torrent_id);

private:
    Ui::MainWindow *ui_;
    std::vector<Torrent> cur_torrens_;

    void read_database(QString file_name);
    void write_database(QString file_name);
    bool check_database(QString file_name);

    int find_index(int torrent_id);
    QString make_progress(size_t percent, size_t index);

};

