#include "irseq.h"
#include <QDebug>

IrSeq::IrSeq(int freq, int type)
{
    this->freq = freq;
    this->type = type;
}


QString IrSeq::getSequence(int addr, int cmd, int num)
{
    QString out;

    switch (this->type) {
    case IR_TYPE_RC5:
        out = getRC5Sequence(addr, cmd, num);
        break;
    case IR_TYPE_NEC:
        out = getNecSamSequence(addr, cmd, IR_TYPE_NEC);
        break;
    case IR_TYPE_SAM:
        out = getNecSamSequence(addr, cmd, IR_TYPE_SAM);
        break;
    default:
        out = QString("Not implemented yet");
        break;
    }

    return out;
}

QString IrSeq::getRC5Sequence(int addr, int cmd, int num)
{
    int T_us = 889;     // base pulse/pause
    int E_ms = 89;      // pause between packets
    int P = this->freq * T_us / 1000000;
    int E = this->freq * E_ms / 1000;

    QString out;
    out.clear();

    int bitSeq = 0x3000 | ((addr & 0x1F) << 6) | (cmd & 0x3F);

    if (num == 2)
        bitSeq |= 0x0800; // toggle bit

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

QString IrSeq::getNecSamSequence(int addr, int cmd, int type)
{
    int startPulse_us = (type == IR_TYPE_NEC ? 9000 : 4500);
    int startPause_us = 4500;
    int pulse_us = 560;
    int pause0_us = 560;
    int pause1_us = 1680;
    int E_ms = 110;      // pause between packets

    int StartPulse = this->freq * startPulse_us / 1000000;
    int StartPause = this->freq * startPause_us / 1000000;
    int pulse = this->freq * pulse_us / 1000000;
    int pause0 = this->freq * pause0_us / 1000000;
    int pause1 = this->freq * pause1_us / 1000000;
    int E = this->freq * E_ms / 1000;

    QString out;
    out.clear();

    out.append(QString::number(this->freq, 10)).append(",");

    // Start sequence
    out.append(QString::number(StartPulse, 10)).append(",");
    out.append(QString::number(StartPause, 10)).append(",");
    // Address
    for (int bit = 0; bit < 8; bit++) {
        out.append(QString::number(pulse, 10)).append(",");
        if (addr & (1 << bit))
            out.append(QString::number(pause1, 10)).append(",");
        else
            out.append(QString::number(pause0, 10)).append(",");
    }
    // Not address
    for (int bit = 0; bit < 8; bit++) {
        out.append(QString::number(pulse, 10)).append(",");
        if (~addr & (1 << bit))
            out.append(QString::number(pause1, 10)).append(",");
        else
            out.append(QString::number(pause0, 10)).append(",");
    }
    // Command
    for (int bit = 0; bit < 8; bit++) {
        out.append(QString::number(pulse, 10)).append(",");
        if (cmd & (1 << bit))
            out.append(QString::number(pause1, 10)).append(",");
        else
            out.append(QString::number(pause0, 10)).append(",");
    }
    // Not command
    for (int bit = 0; bit < 8; bit++) {
        out.append(QString::number(pulse, 10)).append(",");
        if (~cmd & (1 << bit))
            out.append(QString::number(pause1, 10)).append(",");
        else
            out.append(QString::number(pause0, 10)).append(",");
    }

    // Final bit
    out.append(QString::number(pause0, 10)).append(",");
    out.append(QString::number(E, 10));

    return out;
}
