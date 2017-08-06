#include "irseq.h"

IrSeq::IrSeq(int freq, int type)
{
    this->freq = freq;
    this->type = type;
}


QString IrSeq::getSequence(int addr, int cmd)
{
    QString out;

    switch (this->type) {
    case IR_TYPE_RC5:
        out = getRC5Sequence(addr, cmd);
        break;
    default:
        out = QString("Not implemented yet");
        break;
    }

    return out;
}

QString IrSeq::getRC5Sequence(int addr, int cmd)
{
    int Tus = 889;
    int Ems = 89;
    int P = this->freq * Tus / 1000000;
    int E = this->freq * Ems / 1000;

    int bitSeq = 0x3000 | ((addr & 0x1F) << 6) | (cmd & 0x3F);

    QString out;
    out.append(QString::number(this->freq, 10)).append(",");

    // Start bit 1:
    bool oBit = 1;
    bool nBit;

    for (int bit = 12; bit >= 0; bit--) {
        nBit = (bitSeq & (1<<bit) ? true : false);
        if (nBit) {         // New bit is one
            if (oBit) {     // Old bit was also one
                out.append(QString::number(P, 10)).append(","); // old 1
                out.append(QString::number(P, 10)).append(","); // new 0
            } else {
                out.append(QString::number(P * 2, 10).append(",")); // old 0 and new 0
            }
        } else {
            if (oBit) {
                out.append(QString::number(P * 2, 10).append(",")); // old 1 and new 1
            } else {
                out.append(QString::number(P, 10).append(",")); // old 0
                out.append(QString::number(P, 10).append(",")); // new 1
            }
        }
        oBit = nBit;
    }

    // Finish seq
    if (oBit) {
        out.append(QString::number(P, 10)).append(","); // old 1
        out.append(QString::number(E, 10)); // final pause
    } else {
        out.append(QString::number(P + E, 10)); // old 0 and final pause
    }

    return out;
}
