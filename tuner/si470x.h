#ifndef SI470X_H
#define SI470X_H

#include <inttypes.h>

#define SI470X_I2C_ADDR             0x20

// Write mode register values

// 0 byte (02H)
#define SI470X_DSMUTE               0b10000000
#define SI470X_DMUTE                0b01000000
#define SI470X_MONO                 0b00100000
#define SI470X_RDSM                 0b00001000
#define SI470X_SKMODE               0b00000100
#define SI470X_SEEKUP               0b00000010
#define SI470X_SEEK                 0b00000001

// 1 byte (02L)
#define SI470X_DISABLE              0b01000000
#define SI470X_ENABLE               0b00000001

// 2 byte (03H)
#define SI470X_TUNE                 0b10000000
#define SI470X_CHAN_9_8             0b00000011

// 3 byte (03L)
#define SI470X_CHAN_7_0             0b11111111

// 4 byte (04H)
#define SI470X_RDSIEN               0b10000000
#define SI470X_STCIEN               0b01000000
#define SI470X_RDS                  0b00010000
#define SI470X_DE                   0b00001000
#define SI470X_AGCD                 0b00000100

// 5 byte (04L)
#define SI470X_BLNDADJ              0b11000000
    #define SI470X_BLNDADJ_31_49    0b00000000
    #define SI470X_BLNDADJ_37_55    0b01000000
    #define SI470X_BLNDADJ_19_37    0b10000000
    #define SI470X_BLNDADJ_25_43    0b11000000
#define SI470X_GPIO3                0b00110000
#define SI470X_GPIO2                0b00001100
#define SI470X_GPIO1                0b00000011

// 6 byte (05H)
#define SI470X_SEEKTH               0b11111111

// 7 byte (05L)
#define SI470X_BAND                 0b11000000
#define SI470X_BAND_US_EUROPE       0b00000000
#define SI470X_BAND_JAPAN_WIDE      0b01000000
#define SI470X_BAND_JAPAN           0b10000000
#define SI470X_SPACE                0b00110000
#define SI470X_SPACE_200            0b00000000
#define SI470X_SPACE_100            0b00010000
#define SI470X_SPACE_50             0b00100000
#define SI470X_VOLUME               0b00001111

// 8 byte (06H)
#define SI470X_SMUTER               0b11000000
#define SI470X_SMUTEA               0b00110000
#define SI470X_VOLEXT               0b00000001

// 9 byte (06L)
#define SI470X_SKSNR                0b11110000
#define SI470X_SKCNT                0b00001111

// 10 byte (07H)
#define SI470X_XOSCEN               0b10000000
#define SI470X_AHIZEN               0b01000000

// Read mode register values

// 0 byte (0AH)
#define SI740X_RDSR                 0b10000000
#define SI740X_STC                  0b01000000
#define SI740X_SFBL                 0b00100000
#define SI740X_AFCRL                0b00010000
#define SI740X_RDSS                 0b00001000
#define SI740X_BLERA                0b00000110
#define SI740X_ST                   0b00000001

// 1 byte (0AL)
#define SI740X_RSSI                 0b11111111

// 2 byte (0BH)
#define SI740X_BLERB                0b11000000
#define SI740X_BLERC                0b00110000
#define SI740X_BLERD                0b00001100
#define SI740X_READCHAN_9_8         0b00000011

// 3 byte (0BL)
#define SI740X_READCHAN_7_0         0b11111111

// 4 byte (0CH)
#define SI740X_RDSA_HI              0b11111111

// 5 byte (0CL)
#define SI740X_RDSA_LO              0b11111111

// 6 byte (0DH)
#define SI740X_RDSB_HI              0b11111111

// 7 byte (0DL)
#define SI740X_RDSB_LO              0b11111111

// 8 byte (0EH)
#define SI740X_RDSC_HI              0b11111111

// 9 byte (0EL)
#define SI740X_RDSC_LO              0b11111111

// 10 byte (0FH)
#define SI740X_RDSD_HI              0b11111111

// 11 byte (0FL)
#define SI740X_RDSD_LO              0b11111111

// Some useful definitions

#define SI470X_BUF_STEREO(buf)      (buf[0] & SI740X_ST)

#define SI470X_RDBUF_SIZE           12

#define SI470X_CHAN_SPACING         10

#define SI470X_VOL_MIN              0
#define SI470X_VOL_MAX              15

void si470xReset();

void si470xInit();

void si470xSetFreq();

void si470xReadStatus();

void si470xSetVolume(int8_t value);
void si470xSetMute(uint8_t value);

void si470xSetMono(uint8_t value);
#ifdef _RDS
void si470xSetRds(uint8_t value);
#endif
void si470xSetPower(uint8_t value);

void si470xSeek(int8_t direction);

#endif // SI470X_H
