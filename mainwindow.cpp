#include "mainwindow.h"

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    dlgSetup = new SetupDialog();

    connect(pbtnSetup, &QPushButton::clicked, dlgSetup, &SetupDialog::exec);
}

MainWindow::~MainWindow()
{
    delete dlgSetup;
}
