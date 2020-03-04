#pragma once


#include <QMainWindow>
#include <vector>


struct Torrent {
    Torrent(QString name, QString name_locate) : name_(name), name_locate_(name_locate) {}
    QString name_;
    QString name_locate_;
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

};

