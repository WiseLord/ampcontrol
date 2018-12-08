#ifndef SI470X_REGS_H
#define SI470X_REGS_H

// Write mode

// 0 byte (02H)
#define SI470X_DSMUTE               0x80
#define SI470X_DMUTE                0x40
#define SI470X_MONO                 0x20
#define SI470X_RDSM                 0x08
#define SI470X_SKMODE               0x04
#define SI470X_SEEKUP               0x02
#define SI470X_SEEK                 0x01

// 1 byte (02L)
#define SI470X_DISABLE              0x40
#define SI470X_ENABLE               0x01

// 2 byte (03H)
#define SI470X_TUNE                 0x80
#define SI470X_CHAN_9_8             0x03

// 3 byte (03L)
#define SI470X_CHAN_7_0             0xFF

// 4 byte (04H)
#define SI470X_RDSIEN               0x80
#define SI470X_STCIEN               0x40
#define SI470X_RDS                  0x10
#define SI470X_DE                   0x08
#define SI470X_AGCD                 0x04

// 5 byte (04L)
#define SI470X_BLNDADJ              0xC0
#define SI470X_BLNDADJ_31_49        0x00
#define SI470X_BLNDADJ_37_55        0x40
#define SI470X_BLNDADJ_19_37        0x80
#define SI470X_BLNDADJ_25_43        0xC0
#define SI470X_GPIO3                0x30
#define SI470X_GPIO2                0x0C
#define SI470X_GPIO1                0x03

// 6 byte (05H)
#define SI470X_SEEKTH               0xFF

// 7 byte (05L)
#define SI470X_BAND                 0xC0
#define SI470X_BAND_US_EUROPE       0x00
#define SI470X_BAND_JAPAN_WIDE      0x40
#define SI470X_BAND_JAPAN           0x80
#define SI470X_SPACE                0x30
#define SI470X_SPACE_200            0x00
#define SI470X_SPACE_100            0x10
#define SI470X_SPACE_50             0x20
#define SI470X_VOLUME               0x0F

// 8 byte (06H)
#define SI470X_SMUTER               0xC0
#define SI470X_SMUTEA               0x30
#define SI470X_VOLEXT               0x01

// 9 byte (06L)
#define SI470X_SKSNR                0xF0
#define SI470X_SKCNT                0x0F

// 10 byte (07H)
#define SI470X_XOSCEN               0x80
#define SI470X_AHIZEN               0x40

// Read mode

// 0 byte (0AH)
#define SI740X_RDSR                 0x80
#define SI740X_STC                  0x40
#define SI740X_SFBL                 0x20
#define SI740X_AFCRL                0x10
#define SI740X_RDSS                 0x08
#define SI740X_BLERA                0x60
#define SI740X_ST                   0x01

// 1 byte (0AL)
#define SI740X_RSSI                 0xFF

// 2 byte (0BH)
#define SI740X_BLERB                0xC0
#define SI740X_BLERC                0x30
#define SI740X_BLERD                0x0C
#define SI740X_READCHAN_9_8         0x03

// 3 byte (0BL)
#define SI740X_READCHAN_7_0         0xFF

// 4 byte (0CH)
#define SI740X_RDSA_HI              0xFF

// 5 byte (0CL)
#define SI740X_RDSA_LO              0xFF

// 6 byte (0DH)
#define SI740X_RDSB_HI              0xFF

// 7 byte (0DL)
#define SI740X_RDSB_LO              0xFF

// 8 byte (0EH)
#define SI740X_RDSC_HI              0xFF

// 9 byte (0EL)
#define SI740X_RDSC_LO              0xFF

// 10 byte (0FH)
#define SI740X_RDSD_HI              0xFF

// 11 byte (0FL)
#define SI740X_RDSD_LO              0xFF

#endif // SI470X_REGS_H
