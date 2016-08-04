#include "mainwindow.h"
#include "setupdialog.h"

#include <QMessageBox>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    dlgSetup = new SetupDialog(this);
    sPort = new QSerialPort(this);

    closePort();

    connect(pbtnSetup, &QPushButton::clicked,
            dlgSetup, &SetupDialog::exec);
    connect(pbtnConnect, &QPushButton::clicked,
            this, &MainWindow::openPort);
    connect(pbtnDisconnect, &QPushButton::clicked,
            this, &MainWindow::closePort);
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

void MainWindow::on_pbtnSTBY_clicked()
{
    sPort->write(QString("RC 00\r\n").toLocal8Bit());
}

void MainWindow::on_pbtnVOLDEC_clicked()
{
    sPort->write(QString("RC 04\r\n").toLocal8Bit());
}

void MainWindow::on_pbtnVOLINC_clicked()
{
    sPort->write(QString("RC 03\r\n").toLocal8Bit());
}
