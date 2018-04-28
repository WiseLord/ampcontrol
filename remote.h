#ifndef RC5_H
#define RC5_H

#include <inttypes.h>
#include "pins.h"

#define RC5_SHORT_MIN   888     // 444 microseconds
#define RC5_SHORT_MAX   2666    // 1333 microseconds
#define RC5_LONG_MIN    2668    // 1334 microseconds
#define RC5_LONG_MAX    4444    // 2222 microseconds

#define RC5_STBT_MASK   0x3000
#define RC5_TOGB_MASK   0x0800
#define RC5_ADDR_MASK   0x07C0
#define RC5_COMM_MASK   0x003F

#define RC5_BUF_EMPTY   0

enum {
    IR_TYPE_RC5,

    IR_TYPE_NONE = 0x0F
};

typedef struct {
    uint8_t ready : 1;
    uint8_t repeat : 1;
    uint8_t type : 6;
    uint8_t address;
    uint8_t command;
} IRData;

void rcInit();

IRData takeIrData();
IRData getIrData();
void setIrData(uint8_t type, uint8_t addr, uint8_t cmd);

#endif // RC5_H
