#ifndef PT232X_H
#define PT232X_H

#include <inttypes.h>

// I2C address
#define PT2322_I2C_ADDR         0b10001000
#define PT2323_I2C_ADDR         0b10010100

enum {
    PT2322_CH_FL = 0,
    PT2322_CH_FR,
    PT2322_CH_CT,
    PT2322_CH_RL,
    PT2322_CH_RR,
    PT2322_CH_SB,

    PT2322_CH_END
};

// I2C function selection
#define PT2322_NO_FUNC          0b00000000

#define PT2322_TRIM_FL          0b00010000
#define PT2322_TRIM_FR          0b00100000
#define PT2322_TRIM_CT          0b00110000
#define PT2322_TRIM_RL          0b01000000
#define PT2322_TRIM_RR          0b01010000
#define PT2322_TRIM_SB          0b01100000

#define PT2322_FUNCTION         0b01110000
#define PT2322_MUTE_ON          0b00001000
#define PT2322_3D_OFF           0b00000100
#define PT2322_TONE_OFF         0b00000010

#define PT2322_BASS             0b10010000
#define PT2322_MIDDLE           0b10100000
#define PT2322_TREBLE           0b10110000
#define PT2322_INPUT_SW         0b11000111
#define PT2322_VOL_LO           0b11010000
#define PT2322_VOL_HI           0b11100000

#define PT2322_CREAR_REGS       0b11111111

#define PT2323_INPUT_SWITCH     0b11000000
#define PT2323_INPUT_ST1        0b00001011
#define PT2323_INPUT_ST2        0b00001010
#define PT2323_INPUT_ST3        0b00001001
#define PT2323_INPUT_ST4        0b00001000
#define PT2323_INPUT_6CH        0b00000111

#define PT2323_MUTE_FL          0b11110000
#define PT2323_MUTE_FR          0b11110010
#define PT2323_MUTE_CT          0b11110100
#define PT2323_MUTE_SB          0b11110110
#define PT2323_MUTE_RL          0b11111000
#define PT2323_MUTE_RR          0b11111010
#define PT2323_UNMUTE_ALL       0b11111110
#define PT2323_MUTE_ALL         0b11111111

#define PT2323_ENH_SURR         0b11010000
#define PT2323_ENH_SURR_OFF     0b00000001
#define PT2323_MIX              0b10010000
#define PT2323_MIX_6DB          0b00000001

// Number of inputs
#define PT2323_IN_CNT           5

void pt232xReset();
void pt2322SetVolume();
void pt2322SetBMT();
void pt2322SetSpeakers();
void pt2323SetInput();
void pt232xSetSndFunc();

#endif // PT232X_H
