#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "ui_setupdialog.h"

#include <QSerialPort>

class SetupDialog : public QDialog, private Ui::SetupDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = 0);

public slots:
    void readSerialPorts();

private:
    void saveSettings();

private slots:
    void readSettings();
    void showPortInfo(int index);
    void applySettings();
};

#endif // SETUPDIALOG_H
