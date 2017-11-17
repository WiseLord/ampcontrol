#include "lcdconverter.h"

#include <QtCore>

LcdConverter::LcdConverter()
{
    {
        // Latin symbols
        for (ushort i = 0; i < 0x80; i++)
            map_ks0066ru[i] = i;

        // Space between chars
        map_ks0066ru[0x200A] = 0x7f;

        // Russian symbols similar to latin
        map_ks0066ru[0x0410] = 'A';
        map_ks0066ru[0x0412] = 'B';
        map_ks0066ru[0x0421] = 'C';
        map_ks0066ru[0x0415] = 'E';
        map_ks0066ru[0x041d] = 'H';
        map_ks0066ru[0x041a] = 'K';
        map_ks0066ru[0x041c] = 'M';
        map_ks0066ru[0x041e] = 'O';
        map_ks0066ru[0x0420] = 'P';
        map_ks0066ru[0x0422] = 'T';
        map_ks0066ru[0x0425] = 'X';

        map_ks0066ru[0x0430] = 'a';
        map_ks0066ru[0x042c] = 'b';
        map_ks0066ru[0x0441] = 'c';
        map_ks0066ru[0x0435] = 'e';
        map_ks0066ru[0x043e] = 'o';
        map_ks0066ru[0x0440] = 'p';
        map_ks0066ru[0x0445] = 'x';
        map_ks0066ru[0x0443] = 'y';

        // Other russian symbols
        map_ks0066ru[0x0411] = 0xa0; // Б
        map_ks0066ru[0x0413] = 0xa1; // Г
        map_ks0066ru[0x0401] = 0xa2; // Ё
        map_ks0066ru[0x0416] = 0xa3; // Ж
        map_ks0066ru[0x0417] = 0xa4; // З
        map_ks0066ru[0x0418] = 0xa5; // И
        map_ks0066ru[0x0419] = 0xa6; // Й
        map_ks0066ru[0x041b] = 0xa7; // Л
        map_ks0066ru[0x041f] = 0xa8; // П
        map_ks0066ru[0x0423] = 0xa9; // У
        map_ks0066ru[0x0424] = 0xaa; // Ф
        map_ks0066ru[0x0427] = 0xab; // Ч
        map_ks0066ru[0x0428] = 0xac; // Ш
        map_ks0066ru[0x042a] = 0xad; // Ъ
        map_ks0066ru[0x042b] = 0xae; // Ы
        map_ks0066ru[0x042d] = 0xaf; // Э

        map_ks0066ru[0x042e] = 0xb0; // Ю
        map_ks0066ru[0x042f] = 0xb1; // Я
        map_ks0066ru[0x0431] = 0xb2; // б
        map_ks0066ru[0x0432] = 0xb3; // в
        map_ks0066ru[0x0433] = 0xb4; // г
        map_ks0066ru[0x0451] = 0xb5; // ё
        map_ks0066ru[0x0436] = 0xb6; // ж
        map_ks0066ru[0x0437] = 0xb7; // з
        map_ks0066ru[0x0438] = 0xb8; // и
        map_ks0066ru[0x0439] = 0xb9; // й
        map_ks0066ru[0x043a] = 0xba; // к
        map_ks0066ru[0x043b] = 0xbb; // л
        map_ks0066ru[0x043c] = 0xbc; // м
        map_ks0066ru[0x043d] = 0xbd; // н
        map_ks0066ru[0x043f] = 0xbe; // п
        map_ks0066ru[0x0442] = 0xbf; // т

        map_ks0066ru[0x0447] = 0xc0; // ч
        map_ks0066ru[0x0448] = 0xc1; // ш
        map_ks0066ru[0x044a] = 0xc2; // ъ
        map_ks0066ru[0x044b] = 0xc3; // ы
        map_ks0066ru[0x044c] = 0xc4; // ь
        map_ks0066ru[0x044d] = 0xc5; // э
        map_ks0066ru[0x044e] = 0xc6; // ю
        map_ks0066ru[0x044f] = 0xc7; // я

        map_ks0066ru[0x0414] = 0xe0; // Д
        map_ks0066ru[0x0426] = 0xe1; // Ц
        map_ks0066ru[0x0429] = 0xe2; // Щ
        map_ks0066ru[0x0434] = 0xe3; // д
        map_ks0066ru[0x0444] = 0xe4; // ф
        map_ks0066ru[0x0446] = 0xe5; // ц
        map_ks0066ru[0x0449] = 0xe6; // щ

        // Belarussian and Ukrainian special symbols
        map_ks0066ru[0x0490] = 0xe7; // Ґ
        map_ks0066ru[0x0407] = 0xe8; // Ї
        map_ks0066ru[0x0404] = 0xe9; // Є
        map_ks0066ru[0x040e] = 0xea; // Ў
        map_ks0066ru[0x0491] = 0xeb; // ґ
        map_ks0066ru[0x0457] = 0xec; // ї
        map_ks0066ru[0x0454] = 0xed; // є
        map_ks0066ru[0x045e] = 0xee; // ў

        // French special symbols
        map_ks0066ru[0x00C2] = 0xC8; // Â
        map_ks0066ru[0x00E2] = 0xC9; // â
        map_ks0066ru[0x00C0] = 0xCA; // À
        map_ks0066ru[0x00E0] = 0xCB; // à
        map_ks0066ru[0x00C9] = 0xD0; // É
        map_ks0066ru[0x00E9] = 0xD1; // é
        map_ks0066ru[0x00CA] = 0xD2; // Ê
        map_ks0066ru[0x00EA] = 0xD3; // ê
        map_ks0066ru[0x00C8] = 0xD4; // È
        map_ks0066ru[0x00E8] = 0xD5; // è
        map_ks0066ru[0x00CB] = 0xA2; // Ë
        map_ks0066ru[0x00EB] = 0xB5; // ë
        map_ks0066ru[0x00CE] = 0xD6; // Î
        map_ks0066ru[0x00EE] = 0xD7; // î
        map_ks0066ru[0x00CF] = 0xE8; // Ï
        map_ks0066ru[0x00EF] = 0xEC; // ï
        map_ks0066ru[0x00D4] = 0xD8; // Ô
        map_ks0066ru[0x00F4] = 0xD9; // ô
        map_ks0066ru[0x00DB] = 0xDA; // Û
        map_ks0066ru[0x00FB] = 0xDB; // û
        map_ks0066ru[0x00D9] = 0xDC; // Ù
        map_ks0066ru[0x00F9] = 0xDD; // ù
        map_ks0066ru[0x00DC] = 0xDE; // Ü
        map_ks0066ru[0x00FC] = 0xDF; // ü
        map_ks0066ru[0x0178] = 0xCE; // Ÿ
        map_ks0066ru[0x00FF] = 0xCF; // ÿ
        map_ks0066ru[0x00C7] = 0xF0; // Ç
        map_ks0066ru[0x00E7] = 0xF1; // ç

        // Some other symbols
        map_ks0066ru[0x00bf] = 0xcd; // ¿
        map_ks0066ru[0x00b0] = 0xef; // °
        map_ks0066ru[0x2588] = 0xff; // █
    }

    {
        // Latin symbols
        for (ushort i = 0; i < 0x80; i++)
            map_cp1251[i] = i;
        // Russian symbols
        for (ushort i = 0xC0; i <= 0xFF; i++)
            map_cp1251[0x0350 + i] = i;

        map_cp1251[0x0402] = 0x80; // Ђ
        map_cp1251[0x0403] = 0x81; // Ѓ
        map_cp1251[0x201A] = 0x82; // ‚
        map_cp1251[0x0453] = 0x83; // ѓ
        map_cp1251[0x201E] = 0x84; // „
        map_cp1251[0x2026] = 0x85; // …
        map_cp1251[0x2020] = 0x86; // †
        map_cp1251[0x2021] = 0x87; // ‡
        map_cp1251[0x20AC] = 0x88; // €
        map_cp1251[0x2030] = 0x89; // ‰
        map_cp1251[0x0409] = 0x8a; // Љ
        map_cp1251[0x2039] = 0x8b; // ‹
        map_cp1251[0x040A] = 0x8c; // Њ
        map_cp1251[0x040C] = 0x8d; // Ќ
        map_cp1251[0x040B] = 0x8e; // Ћ
        map_cp1251[0x040F] = 0x8f; // Џ

        map_cp1251[0x0452] = 0x90; // ђ
        map_cp1251[0x2018] = 0x91; // ‘
        map_cp1251[0x2019] = 0x92; // ’
        map_cp1251[0x201C] = 0x93; // “
        map_cp1251[0x201D] = 0x94; // ”
        map_cp1251[0x2022] = 0x95; // •
        map_cp1251[0x2013] = 0x96; // –
        map_cp1251[0x2014] = 0x97; // —
        map_cp1251[0x0098] = 0x98; //
        map_cp1251[0x2122] = 0x99; //  ™
        map_cp1251[0x0459] = 0x9a; // љ
        map_cp1251[0x203A] = 0x9b; // ›
        map_cp1251[0x045A] = 0x9c; // њ
        map_cp1251[0x045C] = 0x9d; // ќ
        map_cp1251[0x045B] = 0x9e; // ћ
        map_cp1251[0x045F] = 0x9f; // џ

        map_cp1251[0x00A0] = 0xa0; //
        map_cp1251[0x040e] = 0xa1; // Ў
        map_cp1251[0x045e] = 0xa2; // ў
        map_cp1251[0x0408] = 0xa3; // Ј
        map_cp1251[0x00A4] = 0xa4; // ¤
        map_cp1251[0x0490] = 0xa5; // Ґ
        map_cp1251[0x00A6] = 0xa6; // ¦
        map_cp1251[0x00A7] = 0xa7; // §
        map_cp1251[0x0401] = 0xa8; // Ё
        map_cp1251[0x00A9] = 0xa9; // ©
        map_cp1251[0x0404] = 0xaa; // Є
        map_cp1251[0x00AB] = 0xab; // «
        map_cp1251[0x00AC] = 0xac; // ¬
        map_cp1251[0x00AD] = 0xad; //
        map_cp1251[0x00AE] = 0xae; // ®
        map_cp1251[0x0407] = 0xaf; // Ї

        map_cp1251[0x00B0] = 0xb0; // °
        map_cp1251[0x00B1] = 0xb1; // ±
        map_cp1251[0x0406] = 0xb2; // І
        map_cp1251[0x0456] = 0xb3; // і
        map_cp1251[0x0491] = 0xb4; // ґ
        map_cp1251[0x00B5] = 0xb5; // µ
        map_cp1251[0x00B6] = 0xb6; // ¶
        map_cp1251[0x00B7] = 0xb7; // ·
        map_cp1251[0x0451] = 0xb8; // ё
        map_cp1251[0x2116] = 0xb9; // №
        map_cp1251[0x0454] = 0xba; // є
        map_cp1251[0x00BB] = 0xbb; // »
        map_cp1251[0x0458] = 0xbc; // ј
        map_cp1251[0x0405] = 0xbd; // Ѕ
        map_cp1251[0x0455] = 0xbe; // ѕ
        map_cp1251[0x0457] = 0xbf; // ї
    }
}

QByteArray LcdConverter::encode(QString text, int map)
{
    int i = 0;
    QByteArray ret;
    QMap<QChar, char> *pmap;

    switch (map) {
    case MAP_KS0066_RU:
        pmap = &map_ks0066ru;
        break;
    default:
        pmap = &map_cp1251;
        break;
    }

    foreach (QChar qc, text) {
        if (pmap->contains(qc)) {
            ret[i++] = (*pmap)[qc];
        } else {
            ret[i++] = 0xFF;
        }
    }

    return ret;
}

QString LcdConverter::decode(QByteArray ba, int map)
{
    QString str = "";
    QBuffer buffer(&ba);
    char ch;
    QMap<QChar, char> *pmap;

    switch (map) {
    case MAP_KS0066_RU:
        pmap = &map_ks0066ru;
        break;
    case MAP_CP1251:
    default:
        pmap = &map_cp1251;
        break;
    }

    buffer.open(QIODevice::ReadOnly);
    buffer.seek(0);

    while (buffer.pos() < buffer.size()) {
        buffer.getChar(&ch);
        if (pmap->keys(ch).length() > 0)
            str.append(pmap->keys(ch).at(0));
        else
            str.append(0xFF);
    }

    buffer.close();

    return str;
}
