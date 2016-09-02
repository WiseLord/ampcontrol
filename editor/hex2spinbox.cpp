#include "hex2spinbox.h"

Hex2SpinBox::Hex2SpinBox(QWidget *parent) : QSpinBox(parent)
{

}

QString Hex2SpinBox::textFromValue(int val) const
{
    return QString("%1").arg(val, 2 , 16, QChar('0')).toUpper();
}
