#-------------------------------------------------
#
# Project created by QtCreator 2020-03-01T15:25:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = torrent_gui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        main.cpp \
        open_torrent_window.cpp \
        mainwindow.cpp \
        torrent_client.cpp \

HEADERS += \
        open_torrent_window.h \
        mainwindow.h \
        torrent_client.h \

FORMS += \
        open_torrent_window.ui \
        mainwindow.ui

# Client

# SOURCES += \
#         $$PWD/../../client/source/API.cpp \
#         $$PWD/../../client/source/Download.cpp \
#         $$PWD/../../client/source/InfoHelper.cpp \
#         $$PWD/../../client/source/make_torrent.cpp \
#         $$PWD/../../client/source/View.cpp

# INCLUDEPATH += \
#         $$PWD/../../client/include/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
        resource.qrc

unix:!macx: LIBS += -L$$PWD/../../../../../../../usr/local/lib/ -ltorrent-rasterbar

INCLUDEPATH += $$PWD/../../../../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../../../../usr/local/include

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../../../usr/local/lib/libtorrent-rasterbar.a

unix:!macx: LIBS += -L$$PWD/../../../../../../../usr/lib/x86_64-linux-gnu/ -lboost_system

INCLUDEPATH += $$PWD/../../../../../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../../../../../usr/lib/x86_64-linux-gnu

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../../../usr/lib/x86_64-linux-gnu/libboost_system.a
