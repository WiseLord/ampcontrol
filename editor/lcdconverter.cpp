#include "lcdconverter.h"

#include <QChar>

LcdConverter::LcdConverter()
{

}

QByteArray LcdConverter::encode(QString text)
{
    int i = 0;
    QByteArray ret;

    foreach (QChar qc, text) {
        if (map_ks0066ru.contains(qc)) {
            ret[i++] = map_ks0066ru[qc];
        } else {
            ret[i++] = 0xFF;
        }
    }

    return ret;
}

