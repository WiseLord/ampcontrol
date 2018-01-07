#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QSerialPort>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMenu>

class SetupDialog;

class MainWindow : public QWidget, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent * event);

private:
    SetupDialog *dlgSetup;
    QSerialPort *sPort;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    int dial;

private slots:
    void openSettings();
    void openPort();
    void closePort();
    void sendRC();
    void changeVolume(int value);

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // MAINWINDOW_H
