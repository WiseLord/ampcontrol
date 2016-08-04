#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "ui_setupdialog.h"

#include <QSerialPort>

class SetupDialog : public QDialog, private Ui::SetupDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString portName;
        QSerialPort::BaudRate baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;
    };

    explicit SetupDialog(QWidget *parent = 0);

    Settings settings() const;

private:
    Settings m_settings;

    void fillSerialParam();
    void saveSettings();

private slots:
    void readSerialPorts();
    void showPortInfo(int index);
    void applySettings();
};

#endif // SETUPDIALOG_H
