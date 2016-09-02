#-------------------------------------------------
#
# Project created by QtCreator 2015-08-12T14:57:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11

TARGET = ampcontrol-m8-editor
TEMPLATE = app

DEFINES += _TEA5767 _RDA580X _TUX032 _LM7001 _RDS
DEFINES += _TDA7439 _TDA731X _TDA7448 _PT232X _TEA63X0 _PGA2310 _RDA580X_AUDIO

SOURCES += main.cpp\
    mainwindow.cpp \
    lcdconverter.cpp \
    aboutdialog.cpp \
    hex2spinbox.cpp

HEADERS  += mainwindow.h \
	lcdconverter.h \
    aboutdialog.h \
    hex2spinbox.h

FORMS    += mainwindow.ui \
    aboutdialog.ui

RESOURCES += \
    res.qrc
