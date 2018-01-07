#include "mainwindow.h"
#include "setupdialog.h"

#include <QMessageBox>
#include <QSettings>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    this->setWindowTitle(APPLICATION_NAME);

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu(this);
    QAction *settingsAction = new QAction(tr("Settings"), this);
    QAction *quitAction = new QAction(tr("Quit app"), this);

    dlgSetup = new SetupDialog(this);
    sPort = new QSerialPort(this);

    closePort();

    trayIcon->setToolTip(APPLICATION_NAME);

    trayMenu->addAction(settingsAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

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

    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    connect(dlVolume, &QDial::valueChanged,
            this, &MainWindow::changeVolume);
    connect(dlVolume, &QDial::sliderMoved,
            this, &MainWindow::changeVolume);
    this->dial = dlVolume->value();

    if (settings.value(SETTINGS_APP_AUTOCONNECT, false).toBool() == true) {
        openPort();
    }

    if (settings.value(SETTINGS_APP_HIDEONSTART, false).toBool() == false) {
        this->show();
    }
}

MainWindow::~MainWindow()
{
    closePort();

    delete dlgSetup;
}

void MainWindow::openPort()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    QString portName = settings.value(SETTINGS_SERIAL_PORTNAME, "rfcomm0").toString();
    int baudRate = settings.value(SETTINGS_SERIAL_BAUDRATE, QSerialPort::Baud9600).toInt();
    int dataBits = settings.value(SETTINGS_SERIAL_DATABITS, QSerialPort::Data8).toInt();
    int parity = settings.value(SETTINGS_SERIAL_PARITY, QSerialPort::NoParity).toInt();
    int stopBits = settings.value(SETTINGS_SERIAL_STOPBITS, QSerialPort::OneStop).toInt();
    int flowControl = settings.value(SETTINGS_SERIAL_FLOWCTRL, QSerialPort::NoFlowControl).toInt();

    sPort->setPortName(portName);
    sPort->setBaudRate(baudRate);
    sPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    sPort->setParity(static_cast<QSerialPort::Parity>(parity));
    sPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));
    sPort->setFlowControl(static_cast<QSerialPort::FlowControl>(flowControl));

    if (sPort->open(QIODevice::ReadWrite)) {
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

}

void MainWindow::closePort()
{
    if (sPort->isOpen())
        sPort->close();

    pbtnConnect->setEnabled(true);
    pbtnDisconnect->setEnabled(false);
    frmButtons->setEnabled(false);

    trayIcon->setIcon(QIcon(":/icons/res/22_off.png"));
    this->setWindowIcon(QIcon(":/icons/res/48_off.png"));
}

void MainWindow::sendRC()
{
    QString cmd = sender()->property("RC").toString();

    if (!cmd.isEmpty() && sPort->isOpen()) {
        cmd.prepend("RC ").append("\r\n");
        sPort->write(cmd.toLocal8Bit());
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
    sPort->write(cmd.toLocal8Bit());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->isVisible()){
        event->ignore();
        this->hide();
    } else {
        qApp->exit();
    }
}

void MainWindow::openSettings()
{
    dlgSetup->readSerialPorts();
    dlgSetup->exec();
}

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
