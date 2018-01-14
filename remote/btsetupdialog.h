#ifndef BTSETUPDIALOG_H
#define BTSETUPDIALOG_H

#include "ui_btsetupdialog.h"

#include <QtBluetooth>

class BtSetupDialog : public QDialog, private Ui::BtSetupDialog
{
    Q_OBJECT

public:
    explicit BtSetupDialog(QWidget *parent = 0);

    void addDevice(const QBluetoothDeviceInfo &device);

    QList<QBluetoothDeviceInfo> devList;
private:
    void saveSettings();

private slots:
    void applySettings();
};

#endif // BTSETUPDIALOG_H
