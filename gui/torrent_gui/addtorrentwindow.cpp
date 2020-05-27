#include "addtorrentwindow.h"
#include "ui_addtorrentwindow.h"

#include <iostream>

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidgetItem>
#include <QString>


void HandlerSignals::hand(bool bit) {
    api_.picker.setMark(id_, bit);

    auto marks = api_.picker.getMarks();
    for (size_t i = 0; i < marks.size(); i++) {
        check_boxes_[i]->setChecked(marks[i]);
    }
}


AddTorrentWindow::AddTorrentWindow(QWidget *parent, TorrentAPI &api,
                                   QString source, QString save) : QDialog(parent),
                                                                   ui_(new Ui::AddTorrentWindow),
                                                                   api_(api)
{
    ui_->setupUi(this);

    api_.prepareDownload(source.toStdString(), save.toStdString());
    api_.pickDownloadFiles();

    int count_row = api.picker.download_holder.size();
    ui_->table_widget->setRowCount(count_row);

    ui_->table_widget->setColumnWidth(0, 500);
    ui_->table_widget->setColumnWidth(1, 150);

    this->setFixedWidth(this->width());

    for (auto element : api.picker.download_holder) {
        QString text = QString::fromStdString(element.name_);
        for (int i = 1; i < element.level_; i++) {
            text = "|      " + text;
        }
        auto *widget1 = new QWidget();
        auto *label = new QLabel(text);
        auto *layout1 = new QHBoxLayout(widget1);
        layout1->addWidget(label);
        ui_->table_widget->setCellWidget(check_boxes_.size(), 0, widget1);

        auto *widget2 = new QWidget();
        auto *check_box = new QCheckBox();
        check_box->setCheckState(Qt::Unchecked);
        auto *layout2 = new QHBoxLayout(widget2);
        layout2->addWidget(check_box);
        layout2->setAlignment(Qt::AlignCenter);
        layout2->setContentsMargins(0, 0, 0, 0);
        ui_->table_widget->setCellWidget(check_boxes_.size(), 1, widget2);

        auto *handler = new HandlerSignals(check_boxes_.size(), api_, check_boxes_);
        connect(check_box, SIGNAL(clicked(bool)), handler, SLOT(hand(bool)));

        check_boxes_.push_back(check_box);
    }

}


AddTorrentWindow::~AddTorrentWindow() {
    delete ui_;
}


bool AddTorrentWindow::is_good() {
    return good_;
}


void AddTorrentWindow::on_push_button_add_clicked() {
    good_ = true;
    close();
}

void AddTorrentWindow::on_push_button_close_clicked() {
    close();
}
