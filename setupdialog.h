#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "ui_setupdialog.h"

class SetupDialog : public QDialog, private Ui::SetupDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = 0);
};

#endif // SETUPDIALOG_H
