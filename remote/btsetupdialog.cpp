#include "btsetupdialog.h"
#include "defines.h"

#include <QLabel>
#include <QCheckBox>
#include "btitem.h"

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

    QWidget *itemWgt = new QWidget;
    QLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    BtItem *btItem = new BtItem();
    btItem->setBtName(device.name());
    btItem->setBtAddress(device.address().toString());

    layout->addWidget(btItem);
    itemWgt->setLayout(layout);

    QListWidgetItem *item = new QListWidgetItem(lwDevices);
    item->setSizeHint(itemWgt->sizeHint());
    lwDevices->setItemWidget(item, itemWgt);
}

void BtSetupDialog::saveSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);

    if (lwDevices->selectedItems().count()) {
        QWidget *wgt = lwDevices->itemWidget(lwDevices->selectedItems().at(0));
        if (wgt) {
            BtItem *btItem = wgt->findChild<BtItem *>("BtItem");
            settings.setValue(SETTINGS_BT_DEVICE, btItem->getBtAddress());
        }
    }
}

void BtSetupDialog::applySettings()
{
    saveSettings();
    hide();
}
