#include "mainwindow.h"
#include "defines.h"

#ifdef BLUETOOTH
#include "btsetupdialog.h"
#else
#include "setupdialog.h"
#endif

#include <QMessageBox>
#include <QSettings>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    this->setWindowTitle(APPLICATION_NAME);

#ifdef BLUETOOTH
    dlgSetup = new BtSetupDialog(this);
    port = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    agent = new QBluetoothDeviceDiscoveryAgent;
    connect(agent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
    agent->start();
#else
    dlgSetup = new SetupDialog(this);
    port = new QSerialPort(this);

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu(this);
    QAction *settingsAction = new QAction(tr("Settings"), this);
    QAction *quitAction = new QAction(tr("Quit app"), this);
#endif

    closePort();

#ifdef BLUETOOTH
#else
    trayIcon->setToolTip(APPLICATION_NAME);

    trayMenu->addAction(settingsAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
#endif

    connect(pbtnSetup, SIGNAL(clicked()),
            this, SLOT(openSettings()));

    connect(pbtnConnect, &QPushButton::clicked,
            this, &MainWindow::openPort);
    connect(pbtnDisconnect, &QPushButton::clicked,
            this, &MainWindow::closePort);

    foreach (QPushButton *rcBtn, frmButtons->findChildren<QPushButton*>()) {
        connect(rcBtn, &QPushButton::clicked,
                this, &MainWindow::sendRC);
    }

    connect(dlVolume, &QDial::valueChanged,
            this, &MainWindow::changeVolume);
    connect(dlVolume, &QDial::sliderMoved,
            this, &MainWindow::changeVolume);
    this->dial = dlVolume->value();

#ifdef BLUETOOTH
    this->showFullScreen();
#else
    startupTimer = new QTimer(this);
    if (settings.value(SETTINGS_APP_AUTOCONNECT, false).toBool() == true) {
        startupTimer->setSingleShot(true);
        connect(startupTimer, SIGNAL(timeout()), this, SLOT(openPort()));
        startupTimer->start(settings.value(SETTINGS_APP_AUTOCONNTIME, 0).toInt() * 1000 + 1);
    }

    if (settings.value(SETTINGS_APP_HIDEONSTART, false).toBool() == false) {
        this->show();
    }
#endif
}

MainWindow::~MainWindow()
{
    closePort();
}

void MainWindow::openPort()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

#ifdef BLUETOOTH
    QString device = settings.value(SETTINGS_BT_DEVICE, "00:01:02:03:04:05").toString();

    static const QString serviceUuid(QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));

    for (int i = 0; i < dlgSetup->devList.count(); i++) {
        QBluetoothDeviceInfo dev = dlgSetup->devList[i];
        if (device == dev.address().toString()) {
            port->connectToService(QBluetoothAddress(device), QBluetoothUuid(serviceUuid),
                                     QIODevice::ReadWrite);
            pbtnConnect->setEnabled(false);
            pbtnDisconnect->setEnabled(true);
            frmButtons->setEnabled(true);
        }
    }

#else
    QString portName = settings.value(SETTINGS_SERIAL_PORTNAME, "rfcomm0").toString();
    int baudRate = settings.value(SETTINGS_SERIAL_BAUDRATE, QSerialPort::Baud9600).toInt();
    int dataBits = settings.value(SETTINGS_SERIAL_DATABITS, QSerialPort::Data8).toInt();
    int parity = settings.value(SETTINGS_SERIAL_PARITY, QSerialPort::NoParity).toInt();
    int stopBits = settings.value(SETTINGS_SERIAL_STOPBITS, QSerialPort::OneStop).toInt();
    int flowControl = settings.value(SETTINGS_SERIAL_FLOWCTRL, QSerialPort::NoFlowControl).toInt();

    port->setPortName(portName);
    port->setBaudRate(baudRate);
    port->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    port->setParity(static_cast<QSerialPort::Parity>(parity));
    port->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));
    port->setFlowControl(static_cast<QSerialPort::FlowControl>(flowControl));

    if (port->open(QIODevice::ReadWrite)) {
        trayIcon->showMessage(APPLICATION_NAME,
                              QString(tr("Connected to port")).append(" ").append(portName),
                              QSystemTrayIcon::MessageIcon::Information,
                              2000);
        pbtnConnect->setEnabled(false);
        pbtnDisconnect->setEnabled(true);
        frmButtons->setEnabled(true);

        trayIcon->setIcon(QIcon(":/icons/res/22_on.png"));
        this->setWindowIcon(QIcon(":/icons/res/48_on.png"));
    } else {
        trayIcon->showMessage(APPLICATION_NAME,
                              QString(tr("Can't connect to port")).append(" ").append(portName),
                              QSystemTrayIcon::MessageIcon::Critical,
                              3000);
    }
#endif
}

void MainWindow::closePort()
{
    pbtnConnect->setEnabled(true);
    pbtnDisconnect->setEnabled(false);
    frmButtons->setEnabled(false);

#ifdef BLUETOOTH
    port->disconnectFromService();
#else
    if (port->isOpen())
        port->close();

    trayIcon->setIcon(QIcon(":/icons/res/22_off.png"));
    this->setWindowIcon(QIcon(":/icons/res/48_off.png"));
#endif
}

void MainWindow::sendRC()
{
    QString cmd = sender()->property("RC").toString();
    cmd.prepend("RC ").append("\r\n");

    if (!cmd.isEmpty() && port->isOpen()) {
        port->write(cmd.toLocal8Bit());
    }
}

void MainWindow::changeVolume(int value)
{
    // Some magic to find direction of dial
    int diff =  (value + 63 - this->dial) % 63;
    this->dial = value;

    if (diff > 31)
        diff -= 63;

    QString cmd = "";

    if (diff > 0) {
        cmd = "03";
    } else if (diff < 0) {
        cmd = "04";
    }

    cmd.prepend("RC ").append("\r\n");
    port->write(cmd.toLocal8Bit());
}

#ifdef BLUETOOTH
#else
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();
    } else {
        qApp->exit();
    }
}
#endif

void MainWindow::openSettings()
{
#ifndef BLUETOOTH
    dlgSetup->readSerialPorts();
#endif
    dlgSetup->showFullScreen();
//    dlgSetup->exec();
}

#ifdef BLUETOOTH
void MainWindow::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    dlgSetup->addDevice(device);
    openPort();
}
#else

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        this->setVisible(!this->isVisible());
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    default:
        break;
    }
}
#endif
