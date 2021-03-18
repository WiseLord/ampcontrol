#ifndef LCDCONVERTER_H
#define LCDCONVERTER_H

#include <QString>
#include <QMap>

class LcdConverter
{
public:
    LcdConverter();

    enum {
        MAP_KS0066_RU,
        MAP_CP1251,

        MAP_END
    };

    QByteArray encode(QString text, int map);
    QString decode(QByteArray ba, int map);

private:
    QMap<QChar, char> map_ks0066ru;
    QMap<QChar, char> map_cp1251;
};

#endif // LCDCONVERTER_H
