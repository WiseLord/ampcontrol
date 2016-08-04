#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "setupdialog.h"

class MainWindow : public QWidget, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    SetupDialog *setupDialog;

};

#endif // MAINWINDOW_H
