#include "btitem.h"

BtItem::BtItem(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
}

QString BtItem::getBtName() const
{
    return lblName->text();
}

void BtItem::setBtName(const QString &value)
{
    lblName->setText(value);
}

QString BtItem::getBtAddress() const
{
    return lblAddress->text();
}

void BtItem::setBtAddress(const QString &value)
{
    lblAddress->setText(value);
}
