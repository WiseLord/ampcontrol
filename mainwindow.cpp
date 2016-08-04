#include "mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    setupDialog = new SetupDialog();

    connect(pushButtonSetup, &QPushButton::clicked, setupDialog, &SetupDialog::show);
}

MainWindow::~MainWindow()
{
    delete setupDialog;
}
