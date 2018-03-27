#ifndef RDA5807X_H
#define RDA5807X_H

#include <inttypes.h>

#define RDA5807M_I2C_SEQ_ADDR       0x20
#define RDA5807M_I2C_RAND_ADDR      0x22

// Write mode register values

// 0 register (02H)
#define RDA580X_DHIZ                0b10000000 // Audio out enable (1) / High impedance (0)
#define RDA580X_DMUTE               0b01000000 // Audio unmuted (1) / muted (0)
#define RDA580X_MONO                0b00100000 // Mono mode (1) / stereo mode (0)
#define RDA5807_BASS                0b00010000 // Bass boost (1)
#define RDA580X_RCLK_NON_CAL_MODE   0b00001000 // RCLK always on (0)
#define RDA580X_RCLK_DIR_IN_MODE    0b00000100 // RCLK direct input mode (1)
#define RDA580X_SEEKUP              0b00000010 // Seek up (0) / down (1)
#define RDA580X_SEEK                0b00000001 // Stop seek (0) / start seek in SEEKUP direction (1)

// 1 register (02L)
#define RDA580X_SKMODE              0b10000000 // Stop seeking on band limit (1)
#define RDA580X_CLK_MODE            0b01110000 // Select clock mode
#define RDA580X_CLK_MODE_32768      0b00000000 // Select quartz frequency
#define RDA580X_CLK_MODE_12M        0b00010000
#define RDA580X_CLK_MODE_24M        0b01010000
#define RDA580X_CLK_MODE_13M        0b00100000
#define RDA580X_CLK_MODE_26M        0b01100000
#define RDA580X_CLK_MODE_19M2       0b00110000
#define RDA580X_CLK_MODE_38M4       0b01110000
#define RDA5807_RDS_EN              0b00001000 // RDS/RBDS enable (1)
#define RDA5807_NEW_METHOD          0b00000100 // New demodulation method (1)
#define RDA580X_SOFT_RESET          0b00000010 // Reset settings (1)
#define RDA580X_ENABLE              0b00000001 // Power on radio (1)

// 2 register (03H)
#define RDA580X_CHAN_9_2            0b11111111 // CHAN 9..2 bits

// 3 register (03L)
// CHAN 1.. 0 bits
#define RDA580X_CHAN_1_0            0b11000000 // CHAN 1..0 bits
#define RDA5807_DIRECT_MODE         0b00100000 // Direct mode (1), only used when test
#define RDA580X_TUNE                0b00010000 // Tune enable (1)
#define RDA580X_BAND                0b00001100 // Tuner band selection
#define RDA580X_BAND_US_EUROPE      0b00000000 // 87..108 MHz
#define RDA580X_BAND_JAPAN          0b00000100 // 76..97 MHz
#define RDA580X_BAND_WORLDWIDE      0b00001000 // 76..108 MHz
#define RDA580X_BAND_EASTEUROPE     0b00001100 // 65..76 MHz
#define RDA580X_SPACE               0b00000011 // Stations grid selection
#define RDA580X_SPACE_100           0b00000000 // 100kHz step
#define RDA580X_SPACE_200           0b00000001 // 200kHz step
#define RDA580X_SPACE_50            0b00000010 // 50kHz step
#define RDA580X_SPACE_25            0b00000011 // 25kHz step

// 4 register (04H)
#define RDA580X_STCIEN              0b01000000 // Enable low pulse on GPIO2 when interrupt occurs (1)
#define RDA580X_DE                  0b00001000 // De-emphasis 75us (0) / 50us (1)
#define RDA580X_SOFTMUTE_EN         0b00000010 // Softmute enable (1)
#define RDA580X_AFCD                0b00000001 // AFC disable (1)

// 5 register (04L)
#define RDA580X_I2S_ENABLED         0b01000000 // I2S bus enable (1)
#define RDA580X_GPIO3               0b00110000 // GPIO3 control
#define RDA580X_GPIO3_HI_IMP        0b00000000 // GPIO3 high impedance
#define RDA580X_GPIO3_ST_IND        0b00010000 // GPIO3 as stereo indicator
#define RDA580X_GPIO3_LO            0b00100000 // GPIO3 Low level
#define RDA580X_GPIO3_HI            0b00110000 // GPIO3 HIGH level
#define RDA580X_GPIO2               0b00001100 // GPIO2 control
#define RDA580X_GPIO2_HI_IMP        0b00000000 // GPIO2 high impedance
#define RDA580X_GPIO2_ST_IND        0b00000100 // GPIO2 interrupt (INT)
#define RDA580X_GPIO2_LO            0b00001000 // GPIO2 Low level
#define RDA580X_GPIO2_HI            0b00001100 // GPIO2 HIGH level
#define RDA580X_GPIO1               0b00001100 // GPIO1 control
#define RDA580X_GPIO1_HI_IMP        0b00000000 // GPIO1 high impedance
#define RDA580X_GPIO1_ST_IND        0b00000100 // GPIO1 reserved
#define RDA580X_GPIO1_LO            0b00001000 // GPIO1 Low level
#define RDA580X_GPIO1_HI            0b00001100 // GPIO1 HIGH level

// 6 register (05H)
#define RDA5807_INT_MODE            0b10000000 // 5ms interrupt for RDSIEN on RDS ready (0)
#define RDA5807_SEEKTH              0b00001111 // Seek SNR threshold, 4bits, default 1000=32dB

// 7 register (05L)
#define RDA5802_LNA_PORT_SEL        0b11000000 // LNA input port selection bit
#define RDA5802_LNA_PORT_SEL_NO     0b00000000 // no input
#define RDA5802_LNA_PORT_SEL_LNAN   0b01000000 // LNAN
#define RDA580X_LNA_PORT_SEL_LNAP   0b10000000 // LNAP (FMIN on RDA5807_FP)
#define RDA5802_LNA_PORT_SEL_DUAL   0b11000000 // Dual port input
#define RDA5802_LNA_ICSEL_BIT       0b00110000 // LNA working current bit
#define RDA5802_LNA_ICSEL_BIT_1_8   0b00000000 // 1.8mA
#define RDA5802_LNA_ICSEL_BIT_2_1   0b00010000 // 2.1mA
#define RDA5802_LNA_ICSEL_BIT_2_5   0b00100000 // 2.5mA
#define RDA5802_LNA_ICSEL_BIT_3_0   0b00110000 // 3.0mA
#define RDA580X_VOLUME              0b00001111 // 4 bits volume (0000 - muted, 1111 - max)

// 8 register (06H)
#define RDA5807_OPEN_MODE           0b01100000 // 2 bits (11) to open read-only regs for writing
#define RDA580X_I2S_MODE            0b00010000 // I2S master (0) or slave (1) mode
#define RDA5807_SW_LR               0b00001000 // WS relation to l/r channel: ws=0->r,ws=1->l (0) or ws=0->l,ws=1->r (1)
#define RDA5807_SCLK_I_EDGE         0b00000100 // Normal (0) or inverted (1) sclk
#define RDA5807_DATA_SIGNED         0b00000010 // Unsigned (0) or signed (1) 16-bit audio data
#define RDA5807_WL_I_EDGE           0b00000001 // Normal (0) or inverted (1) ws

// 9 register (06L)
#define RDA580X_I2S_WS_CNT          0b11110000 // I2S WS_STEP selection in master mode
#define RDA580X_I2S_WS_CNT_48       0b10000000 // I2S WS_STEP=48 kpbs
#define RDA580X_I2S_WS_CNT_44_1     0b01110000 // I2S WS_STEP=44.1 kbps
#define RDA580X_I2S_WS_CNT_32       0b01100000 // I2S WS_STEP=32 kbps
#define RDA580X_I2S_WS_CNT_24       0b01010000 // I2S WS_STEP=24 kbps
#define RDA580X_I2S_WS_CNT_22_05    0b01000000 // I2S WS_STEP=22.05 kbps
#define RDA580X_I2S_WS_CNT_16       0b00110000 // I2S WS_STEP=16 kbps
#define RDA580X_I2S_WS_CNT_12       0b00100000 // I2S WS_STEP=12 kbps
#define RDA580X_I2S_WS_CNT_11_025   0b00010000 // I2S WS_STEP=11.025 kbps
#define RDA580X_I2S_WS_CNT_8        0b00000000 // I2S WS_STEP=8 kbps (default)
#define RDA5807_WS_O_EDGE           0b00001000 // Invert (1) ws output when as master
#define RDA5807_SCLK_O_EDGE         0b00000100 // Invert (1) sclk output when as master
#define RDA5807_L_DELY              0b00000010 // Left channel data delay 1T (1)
#define RDA5807_R_DELY              0b00000001 // Right channel data delay 1T (1)

// 10 register (07H)
#define RDA5807_TH_SOFRBLEND        0b01111100 // 5 bits for noise soft blend, default 10000
#define RDA5807_65M_50M_MODE        0b00000010 // For BAND=11, 50..76MHz (0), default 1

// 11 register (07L)
#define RDA5807_SEEK_TH_OLD         0b11111100 // 6 bits seek treshold in old seek mode, valid for SKMODE=1
#define RDA5807_SOFTBLEND_EN        0b00000010 // Softblend enable (1)
#define RDA5807_FREQ_MODE           0b00000001 // Manual freq setup (1) for 12,13 regs

// 12 register (08H)
#define RDA5807_DIRECT_FREQ_HI      0b11111111 // High byte of direct freq offset

// 13 register (08L)
#define RDA5807_DIRECT_FREQ_LO      0b11111111 // High byte of direct freq offset

// Read mode register values

// 0 register (0AH)
#define RDA5807_RDSR                0b10000000 // RDS ready (1)
#define RDA580X_STC                 0b01000000 // Seek/tune complete (1)
#define RDA580X_SF                  0b00100000 // Seek failure (1)
#define RDA5807_RDSS                0b00010000 // RDS decoder synchronized (1)
#define RDA5807_BLK_E               0b00001000 // (When RDS enabled) block E has been found (1)
#define RDA580X_ST                  0b00000100 // Stereo indicator (1)
#define RDA580X_READCHAN_9_8        0b00000011 // READCHAN 9..8 bits

// 1 register (0AL)
#define RDA580X_READCHAN_7_0        0b11111111 // READCHAN 7..0 bits

// 2 register (0BH)
#define RDA580X_RSSI                0b11111110 // 7 bits of RSSI signal level
#define RDA580X_FM_TRUE             0b00000001 // Current channel is a station (1)

// 3 register (0BL)
#define RDA580X_FM_READY            0b10000000 // Ready
#define RDA5807_ABCD_E              0b00010000 // Data block E (1) or blocks A-D (0)
#define RDA5807_BLERA               0b00001100 // 2 bits error level in block A(RDS) or E(RBDS)
#define RDA5807_BLERB               0b00000011 // 2 bits error level in block B(RDS) or E(RBDS)

// 4 register (0CH)
#define RDA5807_RDSA_HI             0b11111111

// 5 register (0CL)
#define RDA5807_RDSA_LO             0b11111111

// 6 register (0DH)
#define RDA5807_RDSB_HI             0b11111111

// 7 register (0DL)
#define RDA5807_RDSB_LO             0b11111111

// 8 register (0EH)
#define RDA5807_RDSC_HI             0b11111111

// 9 register (0EL)
#define RDA5807_RDSC_LO             0b11111111

// 10 register (0FH)
#define RDA5807_RDSD_HI             0b11111111

// 11 register (0FL)
#define RDA5807_RDSD_LO             0b11111111

// Some useful definitions

#define RDA5807_BUF_READY(buf)      (buf[3] & RDA580X_FM_READY)
#define RDA5807_BUF_STEREO(buf)     (buf[0] & RDA580X_ST)

#define RDA5807_RDBUF_SIZE          12

#define RDA5807_CHAN_SPACING        10

#define RDA5807_VOL_MIN             0
#define RDA5807_VOL_MAX             16

void rda580xInit();

void rda580xSetFreq();

void rda580xReadStatus();

void rda580xSetVolume(int8_t value);
void rda580xSetMute(uint8_t value);
void rda580xSetBass(uint8_t value);

void rda580xSetMono(uint8_t value);
#ifdef _RDS
void rda580xSetRds(uint8_t value);
#endif
void rda580xSetPower(uint8_t value);

void rda580xSeek(int8_t direction);

#endif // RDA5807X_H
