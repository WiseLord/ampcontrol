#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "lcdconverter.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void updateHexTable();

private:
    LcdConverter *lc;
    QString fileName;
    QByteArray eep;

private slots:
    void openEeprom();
    void readEep();

    void updateTranslation(int row, int column);

    void setAudioproc(int proc);
};

#endif // MAINWINDOW_H
