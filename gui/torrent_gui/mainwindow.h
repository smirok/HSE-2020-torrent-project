#pragma once


#include <QMainWindow>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_add_torrent_button_clicked();

private:
    Ui::MainWindow *ui;

};

