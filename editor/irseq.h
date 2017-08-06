#ifndef IRSEQ_H
#define IRSEQ_H

#include <QString>

class IrSeq
{
public:
    IrSeq(int freq, int type);

    // Remote control types
    enum {
        IR_TYPE_RC5,
        IR_TYPE_NEC,
        IR_TYPE_RC6,
        IR_TYPE_SAM,

        IR_TYPE_NONE = 0x0F
    };

    QString getSequence(int addr, int cmd);

private:
    int freq;
    int type;

    QString getRC5Sequence(int addr, int cmd);
};

#endif // IRSEQ_H
