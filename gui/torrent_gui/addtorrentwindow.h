#pragma once

#include "API.h"

#include <vector>

#include <QDialog>
#include <QCheckBox>
#include <QObject>


namespace Ui {
class AddTorrentWindow;
}


class HandlerSignals : public QObject {
    Q_OBJECT

public:
    HandlerSignals(int id, API &api, std::vector<QCheckBox *> &check_boxes) : id_(id), api_(api), check_boxes_(check_boxes) {}

public slots:
    void hand(bool bit);

private:
    int id_;
    API &api_;
    std::vector<QCheckBox *> &check_boxes_;

};


class AddTorrentWindow : public QDialog {
    Q_OBJECT

public:
    AddTorrentWindow(QWidget *parent, API &api, QString source, QString save);
    ~AddTorrentWindow();

    bool is_good();

private slots:
    void on_push_button_add_clicked();
    void on_push_button_close_clicked();

private:
    Ui::AddTorrentWindow *ui_;
    API &api_;
    std::vector<QCheckBox *> check_boxes_;

    bool good_ = false;

};
