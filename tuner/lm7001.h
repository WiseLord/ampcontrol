#ifndef LM7001_H
#define LM7001_H

#include <inttypes.h>

// IF = 10.7MHz and step = 50kHz
#define LM7001_IF           1070
#define LM7001_RF           5

// Control byte
#define LM7001_CTRL_B0      (1<<0)
#define LM7001_CTRL_B1      (1<<1)
#define LM7001_CTRL_B2      (1<<2)
#define LM7001_CTRL_TB      (1<<3)
#define LM7001_CTRL_R0      (1<<4)
#define LM7001_CTRL_R1      (1<<5)
#define LM7001_CTRL_R2      (1<<6)
#define LM7001_CTRL_S       (1<<7)

// Set control byte to FM input with 50kHz Fref, (TB = 0, Bi = 0b000, Ri = 0b100, S = 1)
#define LM7001_CTRL_WORD    (LM7001_CTRL_S | LM7001_CTRL_R2)

void lm7001Init();
void lm7001SetFreq();

#endif // LM7001_H
