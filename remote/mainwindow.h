#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#ifdef BLUETOOTH
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#else
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMenu>
#include <QTimer>
#include <QSerialPort>
#endif

#ifdef BLUETOOTH
class BtSetupDialog;
#else
class SetupDialog;
#endif

class MainWindow : public QWidget, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
#ifdef BLUETOOTH
#else
    void closeEvent(QCloseEvent *event);
#endif

private:
#ifdef BLUETOOTH
    BtSetupDialog *dlgSetup;

    QBluetoothDeviceDiscoveryAgent *agent;
    QBluetoothSocket *port;
#else
    SetupDialog *dlgSetup;
    QSerialPort *port;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QTimer *startupTimer;
#endif

    int dial;

private slots:
    void openSettings();
    void openPort();
    void closePort();
    void sendRC();
    void changeVolume(int value);

#ifdef BLUETOOTH
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
#else
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
#endif
};

#endif // MAINWINDOW_H
