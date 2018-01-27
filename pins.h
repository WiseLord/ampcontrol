#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)         x ## y

#define DDR(x)              CONCAT(DDR,x)
#define PORT(x)             CONCAT(PORT,x)
#define PIN(x)              CONCAT(PIN,x)

#if defined(_atmega32)
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
#elif defined(_atmega328p)
// Display data port
#define DISP_D0             D
#define DISP_D0_LINE        (1<<6)
#define DISP_D1             D
#define DISP_D1_LINE        (1<<7)
#define DISP_D2             B
#define DISP_D2_LINE        (1<<0)
#define DISP_D3             B
#define DISP_D3_LINE        (1<<1)
#define DISP_D4             B
#define DISP_D4_LINE        (1<<2)
#define DISP_D5             B
#define DISP_D5_LINE        (1<<3)
#define DISP_D6             B
#define DISP_D6_LINE        (1<<4)
#define DISP_D7             B
#define DISP_D7_LINE        (1<<5)
// Display control port
#define DISP_DATA           C
#define DISP_DATA_LINE      (1<<1)
#define DISP_RW             C
#define DISP_RW_LINE        (1<<1)
#define DISP_STROB          C
#define DISP_STROB_LINE     (1<<0)
#define DISP_CTRL1          D
#define DISP_CTRL1_LINE     (1<<4)
#define DISP_CTRL2          D
#define DISP_CTRL2_LINE     (1<<3)
#define DISP_RESET          C
#define DISP_RESET_LINE     (1<<1)
// Display backlight port
#define DISP_BCKL           D
#define DISP_BCKL_LINE      (1<<5)
#endif

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

// KS0108 Data port
#define KS0108_D0           DISP_D0
#define KS0108_D0_LINE      DISP_D0_LINE
#define KS0108_D1           DISP_D1
#define KS0108_D1_LINE      DISP_D1_LINE
#define KS0108_D2           DISP_D2
#define KS0108_D2_LINE      DISP_D2_LINE
#define KS0108_D3           DISP_D3
#define KS0108_D3_LINE      DISP_D3_LINE
#define KS0108_D4           DISP_D4
#define KS0108_D4_LINE      DISP_D4_LINE
#define KS0108_D5           DISP_D5
#define KS0108_D5_LINE      DISP_D5_LINE
#define KS0108_D6           DISP_D6
#define KS0108_D6_LINE      DISP_D6_LINE
#define KS0108_D7           DISP_D7
#define KS0108_D7_LINE      DISP_D7_LINE
// KS0108 control port
#define KS0108_DI           DISP_DATA
#define KS0108_DI_LINE      DISP_DATA_LINE
#define KS0108_RW           DISP_RW
#define KS0108_RW_LINE      DISP_RW_LINE
#define KS0108_E            DISP_STROB
#define KS0108_E_LINE       DISP_STROB_LINE
#define KS0108_CS1          DISP_CTRL1
#define KS0108_CS1_LINE     DISP_CTRL1_LINE
#define KS0108_CS2          DISP_CTRL2
#define KS0108_CS2_LINE     DISP_CTRL2_LINE
#define KS0108_RES          DISP_RESET
#define KS0108_RES_LINE     DISP_RESET_LINE
// KS0108 Backlight port
#define KS0108_BCKL         DISP_BCKL
#define KS0108_BCKL_LINE    DISP_BCKL_LINE

// ST7920 Data port
#define ST7920_D0           DISP_D0
#define ST7920_D0_LINE      DISP_D0_LINE
#define ST7920_D1           DISP_D1
#define ST7920_D1_LINE      DISP_D1_LINE
#define ST7920_D2           DISP_D2
#define ST7920_D2_LINE      DISP_D2_LINE
#define ST7920_D3           DISP_D3
#define ST7920_D3_LINE      DISP_D3_LINE
#define ST7920_D4           DISP_D4
#define ST7920_D4_LINE      DISP_D4_LINE
#define ST7920_D5           DISP_D5
#define ST7920_D5_LINE      DISP_D5_LINE
#define ST7920_D6           DISP_D6
#define ST7920_D6_LINE      DISP_D6_LINE
#define ST7920_D7           DISP_D7
#define ST7920_D7_LINE      DISP_D7_LINE
// ST7920 control port
#define ST7920_RS           DISP_DATA
#define ST7920_RS_LINE      DISP_DATA_LINE
#define ST7920_RW           DISP_RW
#define ST7920_RW_LINE      DISP_RW_LINE
#define ST7920_E            DISP_STROB
#define ST7920_E_LINE       DISP_STROB_LINE
#define ST7920_PSB          DISP_CTRL1
#define ST7920_PSB_LINE     DISP_CTRL1_LINE
#define ST7920_RST          DISP_RESET
#define ST7920_RST_LINE     DISP_RESET_LINE
// ST7920 Backlight port
#define ST7920_BCKL         DISP_BCKL
#define ST7920_BCKL_LINE    DISP_BCKL_LINE

// LS020 display
#define LS020_DPORT         DISP_D0
#define LS020_RS_LINE       DISP_D1_LINE
#define LS020_RES_LINE      DISP_D0_LINE
#define LS020_CS_LINE       DISP_D4_LINE // Hardware !SS
#define LS020_CLK_LINE      DISP_D7_LINE // Hardware SCK
#define LS020_DAT_LINE      DISP_D5_LINE // Hardware MOSI
// LS020 Backlight port
#define LS020_BCKL          DISP_BCKL
#define LS020_BCKL_LINE     DISP_BCKL_LINE

// Remote control definitions
#if defined(_atmega32)
#define RC                  D
#define RC_LINE             (1<<3)
#elif defined(_atmega328p)
#define RC                  D
#define RC_LINE             (1<<2)
#endif

// Standby/Mute port definitions
#if defined(_atmega32)
#define STMU_MUTE           C
#define STMU_MUTE_LINE      (1<<5)
#define STMU_STBY           C
#define STMU_STBY_LINE      (1<<6)
#elif defined(_atmega328p)
#define STMU_MUTE           C
#define STMU_MUTE_LINE      (1<<2)
#define STMU_STBY           C
#define STMU_STBY_LINE      (1<<3)
#endif

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

// Ext function options
enum {
    USE_DS18B20 = 0,
    USE_SW_SPI,
    USE_INPUT_STATUS,

    USE_EXT_END
};

// EXT outputs definitions
#define EXT_0               C
#define EXT_0_LINE          (1<<2)
#define EXT_1               C
#define EXT_1_LINE          (1<<3)
#define EXT_2               C
#define EXT_2_LINE          (1<<4)

// 1-wire definitions
#define ONE_WIRE            EXT_0
#define ONE_WIRE_LINE       EXT_0_LINE
#define FAN1                EXT_1
#define FAN1_WIRE           EXT_1_LINE
#define FAN2                EXT_2
#define FAN2_WIRE           EXT_2_LINE

// Software SPI definitions
#define SPISW_DI            EXT_0
#define SPISW_DI_LINE       EXT_0_LINE
#define SPISW_CLK           EXT_1
#define SPISW_CLK_LINE      EXT_1_LINE
#define SPISW_CE            EXT_2
#define SPISW_CE_LINE       EXT_2_LINE

// SSD1306 port
#define SSD1306_SCK         DISP_STROB
#define SSD1306_SCK_LINE    DISP_STROB_LINE
#define SSD1306_SDA         DISP_DATA
#define SSD1306_SDA_LINE    DISP_DATA_LINE

#endif // PINS_H
