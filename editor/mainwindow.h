#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "lcdconverter.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

private:
    LcdConverter *lc;
    QString fileName;
    QByteArray eeprom;

private slots:
    void openEeprom();

    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
