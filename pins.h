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

// Display data port
#define DISP_D0             B
#define DISP_D0_LINE        (1<<0)
#define DISP_D1             B
#define DISP_D1_LINE        (1<<1)
#define DISP_D2             B
#define DISP_D2_LINE        (1<<2)
#define DISP_D3             B
#define DISP_D3_LINE        (1<<3)
#define DISP_D4             B
#define DISP_D4_LINE        (1<<4)
#define DISP_D5             B
#define DISP_D5_LINE        (1<<5)
#define DISP_D6             B
#define DISP_D6_LINE        (1<<6)
#define DISP_D7             B
#define DISP_D7_LINE        (1<<7)
// Display control port
#define DISP_DATA           A
#define DISP_DATA_LINE      (1<<2)
#define DISP_RW             A
#define DISP_RW_LINE        (1<<3)
#define DISP_STROB          A
#define DISP_STROB_LINE     (1<<4)
#define DISP_CTRL1          A
#define DISP_CTRL1_LINE     (1<<5)
#define DISP_CTRL2          A
#define DISP_CTRL2_LINE     (1<<6)
#define DISP_RESET          A
#define DISP_RESET_LINE     (1<<7)
// Display backlight port
#define DISP_BCKL           C
#define DISP_BCKL_LINE      (1<<7)

// KS0066 Data port
#define KS0066_D0           DISP_D0
#define KS0066_D0_LINE      DISP_D0_LINE
#define KS0066_D1           DISP_D1
#define KS0066_D1_LINE      DISP_D1_LINE
#define KS0066_D2           DISP_D2
#define KS0066_D2_LINE      DISP_D2_LINE
#define KS0066_D3           DISP_D3
#define KS0066_D3_LINE      DISP_D3_LINE
#define KS0066_D4           DISP_D4
#define KS0066_D4_LINE      DISP_D4_LINE
#define KS0066_D5           DISP_D5
#define KS0066_D5_LINE      DISP_D5_LINE
#define KS0066_D6           DISP_D6
#define KS0066_D6_LINE      DISP_D6_LINE
#define KS0066_D7           DISP_D7
#define KS0066_D7_LINE      DISP_D7_LINE
// KS0066 control port
#define KS0066_RS           DISP_DATA
#define KS0066_RS_LINE      DISP_DATA_LINE
#define KS0066_RW           DISP_RW
#define KS0066_RW_LINE      DISP_RW_LINE
#define KS0066_E            DISP_STROB
#define KS0066_E_LINE       DISP_STROB_LINE
// KS0066 Backlight port
#define KS0066_BCKL         DISP_BCKL
#define KS0066_BCKL_LINE    DISP_BCKL_LINE

// Remote control definitions
#define RC                  D
#define RC_LINE             (1<<3)

// I2C port definitions
#define I2C_SDA             C
#define I2C_SDA_LINE        (1<<1)
#define I2C_SCL             C
#define I2C_SCL_LINE        (1<<0)

// Standby/Mute port definitions
#define STMU_MUTE           C
#define STMU_MUTE_LINE      (1<<5)
#define STMU_STBY           C
#define STMU_STBY_LINE      (1<<6)

// Encoder definitions
#define ENCODER_A           D
#define ENCODER_A_LINE      (1<<1)
#define ENCODER_B           D
#define ENCODER_B_LINE      (1<<2)

// Buttons definitions
#define BUTTON_1            D
#define BUTTON_1_LINE       (1<<0)
#define BUTTON_2            D
#define BUTTON_2_LINE       (1<<4)
#define BUTTON_3            D
#define BUTTON_3_LINE       (1<<5)
#define BUTTON_4            D
#define BUTTON_4_LINE       (1<<6)
#define BUTTON_5            D
#define BUTTON_5_LINE       (1<<7)

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
