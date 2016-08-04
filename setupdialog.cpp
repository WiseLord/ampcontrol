#include "setupdialog.h"
#include <QDebug>

#include <QSerialPortInfo>

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(cmbxSerialPort, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,  &SetupDialog::showPortInfo);

    readPortInfo();
}

void SetupDialog::readPortInfo()
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
    QStringList list = cmbxSerialPort->itemData(index).toStringList();

    lblSystemLocation->setText("Location: " + list.at(1));
    lblDescription->setText("Description: " + list.at(2));
    lblManufacturer->setText("Manufacturer: " + list.at(3));
    lblSerialNumber->setText("Serial number: " + list.at(4));
    lblVendorID->setText("Vendor ID: " + list.at(5));
    lblProductID->setText("Product ID: " + list.at(6));
}
