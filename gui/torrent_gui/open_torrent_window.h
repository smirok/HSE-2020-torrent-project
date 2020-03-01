#ifndef OPEN_TORRENT_WINDOW_H
#define OPEN_TORRENT_WINDOW_H

#include <QDialog>

namespace Ui {
class OpenTorrentWindow;
}

class OpenTorrentWindow : public QDialog
{
    Q_OBJECT

public:
    explicit OpenTorrentWindow(QWidget *parent = nullptr);
    ~OpenTorrentWindow();

private:
    Ui::OpenTorrentWindow *ui;
};

#endif // OPEN_TORRENT_WINDOW_H
