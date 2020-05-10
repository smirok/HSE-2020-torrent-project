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
        mainwindow.cpp \

HEADERS += \
        mainwindow.h \

FORMS += \
        mainwindow.ui

# Client

SOURCES += \
        $$PWD/../../client/source/API.cpp \
        $$PWD/../../client/source/InfoHelper.cpp \
        $$PWD/../../client/source/View.cpp

INCLUDEPATH += $$PWD/../../client/include/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
        resource.qrc

unix:!macx: LIBS += -L/usr/local/lib/ -ltorrent-rasterbar

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libtorrent-rasterbar.a

unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lboost_system

INCLUDEPATH += /usr/lib/x86_64-linux-gnu
DEPENDPATH += usr/lib/x86_64-linux-gnu

unix:!macx: PRE_TARGETDEPS += /usr/lib/x86_64-linux-gnu/libboost_system.a
