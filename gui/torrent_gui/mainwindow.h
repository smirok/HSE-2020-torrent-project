#pragma once


#include <QMainWindow>
#include <vector>


struct Torrent {
    Torrent(QString name, QString locate) : name_(name), locate_(locate) {}
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

private:
    Ui::MainWindow *ui_;
    std::vector<Torrent> cur_torrens_;

    void read_database(QString file_name);
    void write_database(QString file_name);
    bool check_database(QString file_name);

};

