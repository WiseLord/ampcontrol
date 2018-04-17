#ifndef LC72131_H
#define LC72131_H

#include <inttypes.h>

// IF = 10.7MHz and step = 50kHz
#define LC72131_IF          1070
#define LC72131_RF          5 // 25kHz * 2 (for FMIN)

// I/O mode address
#define LC72131_IO_IN1      0x28
#define LC72131_IO_IN2      0x29
#define LC72131_IO_OUT      0x2A

// IN1 mode control word bytes
#define LC72131_IN1_SNS     (1<<0)
#define LC72131_IN1_DVS     (1<<1)
#define LC72131_IN1_CTE     (1<<2)
#define LC72131_IN1_XS      (1<<3)
#define LC72131_IN1_R0      (1<<4)
#define LC72131_IN1_R1      (1<<5)
#define LC72131_IN1_R2      (1<<6)
#define LC72131_IN1_R3      (1<<7)
// IN1 control word          FM input        | Start counter   | 25kHz Fref                      | XTAL= 7.2MHz
#define LC72131_CTRL_IN1    (LC72131_IN1_DVS | LC72131_IN1_CTE | LC72131_IN1_R1 | LC72131_IN1_R0 | LC72131_IN1_XS)

// IN2 mode control word1 bytes
#define LC72131_IN2_IOC1    (1<<0)
#define LC72131_IN2_IOC2    (1<<1)
#define LC72131_IN2_IO1     (1<<2)
#define LC72131_IN2_IO2     (1<<3)
#define LC72131_IN2_BO1     (1<<4)
#define LC72131_IN2_BO2     (1<<5)
#define LC72131_IN2_BO3     (1<<6)
#define LC72131_IN2_BO4     (1<<7)
// IN2 control word1 bytes   BO4 high
//#define LC72131_CTRL_IN2_1    (LC72131_IN2_BO4 | LC72131_IN2_IO2 | LC72131_IN2_IOC2)
#define LC72131_CTRL_IN2_1  (LC72131_IN2_BO4)
// IN2 mode control word2 bytes
#define LC72131_IN2_DNC     (1<<0)
#define LC72131_IN2_DOC0    (1<<1)
#define LC72131_IN2_DOC1    (1<<2)
#define LC72131_IN2_DOC2    (1<<3)
#define LC72131_IN2_UL0     (1<<4)
#define LC72131_IN2_UL1     (1<<5)
#define LC72131_IN2_DZ0     (1<<6)
#define LC72131_IN2_DZ1     (1<<7)
// IN2 control word2 bytes   DO low when unlock state detected
#define LC72131_CTRL_IN2_2  (LC72131_IN2_DOC0)

// IN2 mode control word3 bytes
#define LC72131_IN2_GT0     (1<<0)
#define LC72131_IN2_GT1     (1<<1)
#define LC72131_IN2_TBC     (1<<2)
#define LC72131_IN2_DLC     (1<<3)
#define LC72131_IN2_IFS     (1<<4)
#define LC72131_IN2_TEST0   (1<<5)
#define LC72131_IN2_TEST1   (1<<6)
#define LC72131_IN2_TEST2   (1<<7)
// IN2 control word3 bytes   8ms / 3..4ms    | IF counter normal mode
#define LC72131_CTRL_IN2_3  (LC72131_IN2_GT0 | LC72131_IN2_IFS)

void lc72131Init();
void lc72131SetFreq();

#endif // LC72131_H
