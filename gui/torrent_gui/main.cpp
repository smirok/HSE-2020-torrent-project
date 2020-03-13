#include "mainwindow.h"
#include <QApplication>
#include "/home/andrey/Документы/cpp_project/HSE-2020-torrent-project/client/include/API.h"


int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
