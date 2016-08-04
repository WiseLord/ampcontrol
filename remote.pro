#-------------------------------------------------
#
# Project created by QtCreator 2016-08-04T10:19:01
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = remote
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    setupdialog.cpp

HEADERS  += mainwindow.h \
    setupdialog.h

FORMS    += mainwindow.ui \
    setupdialog.ui
