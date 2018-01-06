#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "ui_setupdialog.h"

#include <QSerialPort>

#define ORGANIZATION_NAME           "WiseLord"
#define ORGANIZATION_DOMAIN         "github.io"
#define APPLICATION_NAME            "Ampcontrol"

#define SETTINGS_SERIAL             "SerialPort"
#define SETTINGS_SERIAL_PORTNAME    "SerialPort/PortName"
#define SETTINGS_SERIAL_BAUDRATE    "SerialPort/BaudRate"
#define SETTINGS_SERIAL_DATABITS    "SerialPort/DataBits"
#define SETTINGS_SERIAL_PARITY      "SerialPort/Parity"
#define SETTINGS_SERIAL_STOPBITS    "SerialPort/StopBits"
#define SETTINGS_SERIAL_FLOWCTRL    "SerialPort/FlowCtrl"

#define SETTINGS_APP                "Application"
#define SETTINGS_APP_AUTOCONNECT    "Application/Autoconnect"
#define SETTINGS_APP_HIDEONSTART    "Application/HideOnStartup"

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
