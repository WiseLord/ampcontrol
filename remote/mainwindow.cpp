#include "mainwindow.h"
#include "setupdialog.h"

#include <QMessageBox>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu(this);
    QAction *quitAction = new QAction(tr("Quit app"), this);

    trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_MediaPlay));
    trayIcon->setToolTip("Ampcontrol");

    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    dlgSetup = new SetupDialog(this);
    sPort = new QSerialPort(this);

    closePort();

    connect(pbtnSetup, &QPushButton::clicked,
            dlgSetup, &SetupDialog::exec);
    connect(pbtnConnect, &QPushButton::clicked,
            this, &MainWindow::openPort);
    connect(pbtnDisconnect, &QPushButton::clicked,
            this, &MainWindow::closePort);

    foreach (QPushButton *rcBtn, frmButtons->findChildren<QPushButton*>()) {
        connect(rcBtn, &QPushButton::clicked,
                this, &MainWindow::sendRC);
    }

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

MainWindow::~MainWindow()
{
    closePort();

    delete dlgSetup;
}

void MainWindow::openPort()
{
    SetupDialog::Settings cs = dlgSetup->settings();

    sPort->setPortName(cs.portName);
    sPort->setBaudRate(cs.baudRate);
    sPort->setDataBits(cs.dataBits);
    sPort->setParity(cs.parity);
    sPort->setStopBits(cs.stopBits);
    sPort->setFlowControl(cs.flowControl);

    if (sPort->open(QIODevice::ReadWrite)) {
        pbtnConnect->setEnabled(false);
        pbtnDisconnect->setEnabled(true);
        frmButtons->setEnabled(true);
    }

}

void MainWindow::closePort()
{
    if (sPort->isOpen())
        sPort->close();

    pbtnConnect->setEnabled(true);
    pbtnDisconnect->setEnabled(false);
    frmButtons->setEnabled(false);
}

void MainWindow::sendRC()
{
    QString cmd = sender()->property("RC").toString();

    if (!cmd.isEmpty() && sPort->isOpen()) {
        cmd.prepend("RC ").append("\r\n");
        sPort->write(cmd.toLocal8Bit());
    }
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
