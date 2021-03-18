#ifndef HEX2SPINBOX_H
#define HEX2SPINBOX_H

#include <QSpinBox>

class Hex2SpinBox : public QSpinBox
{
    Q_OBJECT

public:

    Hex2SpinBox(QWidget *parent = 0);

    virtual QString textFromValue(int val) const;
};

#endif // HEX2SPINBOX_H
