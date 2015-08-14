#ifndef LCDCONVERTER_H
#define LCDCONVERTER_H

#include <QString>
#include <QMap>

class LcdConverter
{
public:
    LcdConverter();

    QByteArray encode(QString text);
    QString decode(QByteArray ba);

private:
    QMap<QChar, char> map_ks0066ru;
};

#endif // LCDCONVERTER_H
