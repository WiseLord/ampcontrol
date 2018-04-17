#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)         x ## y

#define DDR(x)              CONCAT(DDR,x)
#define PORT(x)             CONCAT(PORT,x)
#define PIN(x)              CONCAT(PIN,x)

#define OUT(x)              (DDR(x) |= x ## _LINE)
#define IN(x)               (DDR(x) &= ~x ## _LINE)
#define SET(x)              (PORT(x) |= x ## _LINE)
#define CLR(x)              (PORT(x) &= ~x ## _LINE)
#define READ(x)             (PIN(x) & x ## _LINE)

// Standby/Mute port definitions
#define STMU_MUTE           C
#define STMU_MUTE_LINE      (1<<5)

#define KS0066_D7           B
#define KS0066_D7_LINE      (1<<7)
#define KS0066_D6           B
#define KS0066_D6_LINE      (1<<6)
#define KS0066_D5           B
#define KS0066_D5_LINE      (1<<5)
#define KS0066_D4           B
#define KS0066_D4_LINE      (1<<4)
#define KS0066_D3           B
#define KS0066_D3_LINE      (1<<3)
#define KS0066_D2           B
#define KS0066_D2_LINE      (1<<2)
#define KS0066_D1           B
#define KS0066_D1_LINE      (1<<1)
#define KS0066_D0           B
#define KS0066_D0_LINE      (1<<0)
#define KS0066_BL           C
#define KS0066_BL_LINE      (1<<7)
#define KS0066_E            A
#define KS0066_E_LINE       (1<<4)
#define KS0066_RW           A
#define KS0066_RW_LINE      (1<<3)
#define KS0066_RS           A
#define KS0066_RS_LINE      (1<<2)

#define BACKLIGHT           C
#define BACKLIGHT_LINE      (1<<7)

// RC5 definitions
#define RC5                 D
#define RC5_LINE            (1<<3)

// I2C port definitions
#define I2C_SDA             C
#define I2C_SDA_LINE        (1<<1)
#define I2C_SCL             C
#define I2C_SCL_LINE        (1<<0)

// EXT outputs definitions
#define EXT_0               C
#define EXT_0_LINE          (1<<2)
#define EXT_1               C
#define EXT_1_LINE          (1<<3)
#define EXT_2               C
#define EXT_2_LINE          (1<<4)

// Software SPI definitions
#define SPISW_DI            EXT_0
#define SPISW_DI_LINE       EXT_0_LINE
#define SPISW_CLK           EXT_1
#define SPISW_CLK_LINE      EXT_1_LINE
#define SPISW_CE            EXT_2
#define SPISW_CE_LINE       EXT_2_LINE

// SI470X Reset pins
#define SI470X_B_SDIO       I2C_SDA
#define SI470X_B_SDIO_LINE  I2C_SDA_LINE
#define SI470X_B_SCLK       I2C_SCL
#define SI470X_B_SCLK_LINE  I2C_SCL_LINE
#define SI470X_B_RST        EXT_2
#define SI470X_B_RST_LINE   EXT_2_LINE

#endif // PINS_H
