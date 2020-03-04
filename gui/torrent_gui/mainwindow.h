#pragma once


#include <QMainWindow>
#include <vector>


class Torrent {
    QString name_;
public:
    Torrent(QString name) : name_(name) {}
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

private:
    Ui::MainWindow *ui_;
    std::vector<Torrent> cur_torrens_;

};

