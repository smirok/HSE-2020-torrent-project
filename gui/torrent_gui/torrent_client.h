#pragma once


#include <QObject>
#include <QString>


class TorrentClient : public QObject {

    Q_OBJECT

public:
    TorrentClient() = default;
    TorrentClient(QString file_name, int id);

public slots:
    void start_load();

signals:
    void send_statistic(int, int);

private:
    QString file_name_ = {};
    int id_;
    int other_ = 0;
};
