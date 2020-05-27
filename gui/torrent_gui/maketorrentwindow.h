#pragma once

#include <QDialog>
#include <QString>
#include <QVector>


namespace Ui {
class MakeTorrentWindow;
}


class MakeTorrentWindow : public QDialog {
    Q_OBJECT

public:
    explicit MakeTorrentWindow(QWidget *parent = nullptr);
    ~MakeTorrentWindow();

    std::string get_save_directory();
    std::string get_source();
    std::vector<std::string> get_trackers();
    std::string get_name();

    bool is_good();

private slots:
    void on_radio_button_folder_clicked();
    void on_radio_button_file_clicked();
    void on_push_button_source_clicked();
    void on_push_button_save_location_clicked();
    void on_push_button_new_clicked();
    void on_push_button_close_clicked();

private:
    Ui::MakeTorrentWindow *ui_;

    QString path_to_save_directory_;
    QString path_to_source_;
    QVector<QString> trackers_;
    QString name_;

    bool good_ = false;

};
