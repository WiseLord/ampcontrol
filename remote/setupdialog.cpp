#include "setupdialog.h"
#include "defines.h"

#include <QSerialPortInfo>
#include <QSettings>

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
    connect(pbtnUndo, &QPushButton::clicked,
            this, &SetupDialog::readSettings);

    readSettings();
}

void SetupDialog::readSettings()
{
    readSerialPorts();

    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    int baudRate = settings.value(SETTINGS_SERIAL_BAUDRATE, QSerialPort::Baud9600).toInt();
    int dataBits = settings.value(SETTINGS_SERIAL_DATABITS, QSerialPort::Data8).toInt();
    int parity = settings.value(SETTINGS_SERIAL_PARITY, QSerialPort::NoParity).toInt();
    int stopBits = settings.value(SETTINGS_SERIAL_STOPBITS, QSerialPort::OneStop).toInt();
    int flowControl = settings.value(SETTINGS_SERIAL_FLOWCTRL, QSerialPort::NoFlowControl).toInt();

    bool autoConnect = settings.value(SETTINGS_APP_AUTOCONNECT, false).toBool();
    int autoConnectTime = settings.value(SETTINGS_APP_AUTOCONNTIME, 0).toInt();
    bool hideOnStart = settings.value(SETTINGS_APP_HIDEONSTART, false).toBool();

    cmbxBaudRate->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    cmbxBaudRate->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    cmbxBaudRate->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    cmbxBaudRate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    cmbxBaudRate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    cmbxBaudRate->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    cmbxBaudRate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    cmbxBaudRate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    cmbxBaudRate->setCurrentIndex(cmbxBaudRate->findData(baudRate));

    cmbxDataBits->addItem(QStringLiteral("5"), QSerialPort::Data5);
    cmbxDataBits->addItem(QStringLiteral("6"), QSerialPort::Data6);
    cmbxDataBits->addItem(QStringLiteral("7"), QSerialPort::Data7);
    cmbxDataBits->addItem(QStringLiteral("8"), QSerialPort::Data8);
    cmbxDataBits->setCurrentIndex(cmbxDataBits->findData(dataBits));

    cmbxParity->addItem(tr("None"), QSerialPort::NoParity);
    cmbxParity->addItem(tr("Even"), QSerialPort::EvenParity);
    cmbxParity->addItem(tr("Odd"), QSerialPort::OddParity);
    cmbxParity->addItem(tr("Space"), QSerialPort::SpaceParity);
    cmbxParity->addItem(tr("Mark"), QSerialPort::MarkParity);
    cmbxParity->setCurrentIndex(cmbxParity->findData(parity));

    cmbxStopBits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    cmbxStopBits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    cmbxStopBits->setCurrentIndex(cmbxStopBits->findData(stopBits));

    cmbxFlow->addItem(tr("None"), QSerialPort::NoFlowControl);
    cmbxFlow->addItem(QStringLiteral("RTS/CTS"), QSerialPort::HardwareControl);
    cmbxFlow->addItem(QStringLiteral("XON/XOFF"), QSerialPort::SoftwareControl);
    cmbxFlow->setCurrentIndex(cmbxFlow->findData(flowControl));

    cbxAutoconnect->setChecked(autoConnect);
    sbAutoconnect->setValue(autoConnectTime);
    cbxHideOnStart->setChecked(hideOnStart);
}

void SetupDialog::saveSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    settings.setValue(SETTINGS_SERIAL_PORTNAME, cmbxSerialPort->currentText());
    settings.setValue(SETTINGS_SERIAL_BAUDRATE, cmbxBaudRate->currentData().toInt());
    settings.setValue(SETTINGS_SERIAL_DATABITS, cmbxDataBits->currentData().toInt());
    settings.setValue(SETTINGS_SERIAL_PARITY,   cmbxParity->currentData().toInt());
    settings.setValue(SETTINGS_SERIAL_STOPBITS, cmbxStopBits->currentData().toInt());
    settings.setValue(SETTINGS_SERIAL_FLOWCTRL, cmbxFlow->currentData().toInt());

    settings.setValue(SETTINGS_APP_AUTOCONNECT, cbxAutoconnect->isChecked());
    settings.setValue(SETTINGS_APP_AUTOCONNTIME, sbAutoconnect->value());
    settings.setValue(SETTINGS_APP_HIDEONSTART, cbxHideOnStart->isChecked());

    settings.sync();
}

void SetupDialog::readSerialPorts()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    QString portName = settings.value(SETTINGS_SERIAL_PORTNAME, "rfcomm0").toString();

    cmbxSerialPort->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        cmbxSerialPort->addItem(info.portName(), info.portName());
    }

    // Try to find index of previously used port
    int savedIndex = cmbxSerialPort->findData(portName);

    if (savedIndex < 0) {
        // Not found, but will add it manually
        cmbxSerialPort->addItem(portName, portName.append(" ").append(tr("(NOT FOUND)")));
    }

    cmbxSerialPort->setCurrentIndex(cmbxSerialPort->findData(portName));
    showPortInfo(cmbxSerialPort->currentIndex());
}

void SetupDialog::showPortInfo(int index)
{
    if (index < 0)
        return;

    QSerialPortInfo portInfo(cmbxSerialPort->itemData(index).toString());

    lblSystemLocation->setText(portInfo.systemLocation());
    lblDescription->setText(portInfo.description());
    lblManufacturer->setText(portInfo.manufacturer());
    lblSerialNumber->setText(portInfo.serialNumber());
    lblVendorID->setText(QString::number(portInfo.vendorIdentifier(), 16));
    lblProductID->setText(QString::number(portInfo.productIdentifier(), 16));
}

void SetupDialog::applySettings()
{
    saveSettings();
    hide();
}
