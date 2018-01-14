#include "btsetupdialog.h"
#include "defines.h"

BtSetupDialog::BtSetupDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(pbOK, &QPushButton::clicked,
            this, &BtSetupDialog::applySettings);
}

void BtSetupDialog::addDevice(const QBluetoothDeviceInfo &device)
{
    devList.append(device);
    lwDevices->addItem(device.address().toString());
}

void BtSetupDialog::saveSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    settings.setValue(SETTINGS_BT_DEVICE, lwDevices->currentItem()->text());
}

void BtSetupDialog::applySettings()
{
    saveSettings();
    hide();
}
