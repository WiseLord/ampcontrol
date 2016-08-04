#include "setupdialog.h"
#include <QDebug>

#include <QSerialPortInfo>

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(cmbxSerialPort, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,  &SetupDialog::showPortInfo);
    connect(pbtnUpdate, &QPushButton::clicked,
            this, &SetupDialog::readSerialPorts);
    connect(pbtnApply, &QPushButton::clicked,
            this, &SetupDialog::applySettings);

    fillSerialParam();
    readSerialPorts();
    saveSettings();
}

SetupDialog::Settings SetupDialog::settings() const
{
    return m_settings;
}

void SetupDialog::fillSerialParam()
{
    cmbxBaudRate->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    cmbxBaudRate->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    cmbxBaudRate->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    cmbxBaudRate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    cmbxBaudRate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    cmbxBaudRate->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    cmbxBaudRate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    cmbxBaudRate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    cmbxBaudRate->setCurrentIndex(3);

    cmbxDataBits->addItem(QStringLiteral("5"), QSerialPort::Data5);
    cmbxDataBits->addItem(QStringLiteral("6"), QSerialPort::Data6);
    cmbxDataBits->addItem(QStringLiteral("7"), QSerialPort::Data7);
    cmbxDataBits->addItem(QStringLiteral("8"), QSerialPort::Data8);
    cmbxDataBits->setCurrentIndex(3);

    cmbxParity->addItem(tr("None"), QSerialPort::NoParity);
    cmbxParity->addItem(tr("Even"), QSerialPort::EvenParity);
    cmbxParity->addItem(tr("Odd"), QSerialPort::OddParity);
    cmbxParity->addItem(tr("Space"), QSerialPort::SpaceParity);
    cmbxParity->addItem(tr("Mark"), QSerialPort::MarkParity);
    cmbxParity->setCurrentIndex(0);

    cmbxStopBits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    cmbxStopBits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    cmbxStopBits->setCurrentIndex(0);

    cmbxFlow->addItem(tr("None"), QSerialPort::NoFlowControl);
    cmbxFlow->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    cmbxFlow->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    cmbxFlow->setCurrentIndex(0);
}

void SetupDialog::saveSettings()
{
    m_settings.portName = cmbxSerialPort->currentText();
    m_settings.baudRate = static_cast<QSerialPort::BaudRate>(cmbxBaudRate->currentData().toInt());
    m_settings.dataBits = static_cast<QSerialPort::DataBits>(cmbxDataBits->currentData().toInt());
    m_settings.parity = static_cast<QSerialPort::Parity>(cmbxParity->currentData().toInt());
    m_settings.stopBits = static_cast<QSerialPort::StopBits>(cmbxStopBits->currentData().toInt());
    m_settings.flowControl = static_cast<QSerialPort::FlowControl>(cmbxFlow->currentData().toInt());
}

void SetupDialog::readSerialPorts()
{
    cmbxSerialPort->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;

        list << info.portName()
             << info.systemLocation()
             << info.description()
             << info.manufacturer()
             << info.serialNumber()
             << QString::number(info.vendorIdentifier())
             << QString::number(info.productIdentifier());

        cmbxSerialPort->addItem(list.at(0), list);
    }

}

void SetupDialog::showPortInfo(int index)
{
    if (index < 0)
        return;

    QStringList list = cmbxSerialPort->itemData(index).toStringList();

    lblSystemLocation->setText(list.at(1));
    lblDescription->setText(list.at(2));
    lblManufacturer->setText(list.at(3));
    lblSerialNumber->setText(list.at(4));
    lblVendorID->setText(list.at(5));
    lblProductID->setText(list.at(6));
}

void SetupDialog::applySettings()
{
    saveSettings();
    hide();
}
