#ifndef BTITEM_H
#define BTITEM_H

#include "ui_btitem.h"

class BtItem : public QWidget, private Ui::BtItem
{
    Q_OBJECT

public:
    explicit BtItem(QWidget *parent = 0);

    QString getBtName() const;
    void setBtName(const QString &value);

    QString getBtAddress() const;
    void setBtAddress(const QString &value);

private:
    QString btName;
    QString btAddress;
};

#endif // BTITEM_H
