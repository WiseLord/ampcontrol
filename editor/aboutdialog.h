#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "ui_aboutdialog.h"

class AboutDialog : public QDialog, private Ui::AboutDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
};

#endif // ABOUTDIALOG_H
