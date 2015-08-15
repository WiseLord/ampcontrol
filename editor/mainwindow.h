#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "lcdconverter.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void updateHexTable(int pos);
    void updateHexTable();

private:
    LcdConverter *lc;
    QString fileName;
    QByteArray eep;

    void setAudioParam(QDoubleSpinBox *spb, double min, double max, double step, int param);

private slots:
    void openEeprom();
    void readEep();

    void updateTranslation(int row, int column);

    void setAudioproc(int proc);
    void setVolume(double value);
    void setBass(double value);
    void setMiddle(double value);
    void setTreble(double value);
    void setPreamp(double value);
    void setFrontrear(double value);
    void setBalance(double value);
    void setCenter(double value);
    void setSubwoofer(double value);
    void setGain0(double value);
    void setGain1(double value);
    void setGain2(double value);
    void setGain3(double value);
    void setInput(int value);
    void setLoudness(int value);
};

#endif // MAINWINDOW_H
