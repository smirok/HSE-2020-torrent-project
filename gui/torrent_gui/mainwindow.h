#pragma once


#include "API.h"

#include <QMainWindow>
#include <QThread>
#include <QListWidgetItem>
#include <QObject>
#include <QProgressBar>

#include <vector>
#include <atomic>


static std::atomic<bool> program_is_running{true};


class Worker : public QObject {
    Q_OBJECT

public slots:
    void start();

signals:
    void update_screen();

};


struct Torrent {
    Torrent(QString name, QString locate, QString marks, QProgressBar *progress_bar) : name_(name), locate_(locate),
                                                                                       marks_(marks), progress_bar_(progress_bar) {}
    QString name_;
    QString locate_;
    QString marks_;
    QProgressBar *progress_bar_;

    bool download_ = true;
    bool finished_ = false;
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
    void on_action_create_torrent_triggered();
    void on_action_delete_torrent_triggered();
    void on_action_pause_torrent_triggered();
    void on_action_start_torrent_triggered();

    void open_in_folder();
    void update_statistic();
    void show_context_menu(const QPoint &point);

    void on_list_cur_torrents_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);



private:
    Ui::MainWindow *ui_;
    std::vector<Torrent> cur_torrens_;
    API api_;
    QThread *thread_;

    void read_database(QString file_name);
    void write_database(QString file_name);
    bool check_database(QString file_name);

};
