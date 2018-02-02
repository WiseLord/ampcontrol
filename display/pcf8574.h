#ifndef PCF8574_H
#define PCF8574_H

#include <avr/io.h>
#include <inttypes.h>

#define PCF8574_ADDR                0x40

#define PCF8574_BCKL                (1<<3)
#define PCF8574_E                   (1<<2)
#define PCF8574_RW                  (1<<1)
#define PCF8574_RS                  (1<<0)

#define KS0066_SCREEN_WIDTH         16
#define KS0066_SCREEN_HEIGHT        2

#define KS0066_CLEAR                0x01
#define KS0066_HOME                 0x02

#define KS0066_SET_MODE             0x04
#define KS0066_INC_ADDR         0x02
#define KS0066_DEC_ADDR         0x00
#define KS0066_SHIFT_WINDOW     0x01

#define KS0066_DISPLAY              0x08
#define KS0066_DISPAY_ON        0x04
#define KS0066_CURSOR_ON        0x02
#define KS0066_CUR_BLINK_ON     0x01

#define KS0066_SHIFT                0x10
#define KS0066_SHIFT_DISPLAY    0x08
#define KS0066_SHIFT_CURSOR     0x00
#define KS0066_SHIFT_RIGHT      0x04
#define KS0066_SHIFT_LEFT       0x00

#define KS0066_FUNCTION             0x20
#define KS0066_8BIT             0x10
#define KS0066_4BIT             0x00
#define KS0066_2LINES           0x08
#define KS0066_1LINE            0x00
#define KS0066_5X10             0x04
#define KS0066_5X8              0x00

#define KS0066_SET_CGRAM            0x40
#define KS0066_SET_DDRAM            0x80
#define KS0066_LINE_WIDTH       0x40

#define KS0066_INIT_DATA            (KS0066_FUNCTION | KS0066_8BIT)

#define KS0066_STA_BUSY             0x80

#define ks0066WriteChar(x) ks0066WriteData(x)

void ks0066WriteCommand(uint8_t command);
void ks0066WriteData(uint8_t data);

void ks0066Clear(void);
void ks0066Init(void);

void ks0066SetXY(uint8_t x, uint8_t y);
void ks0066WriteString(uint8_t *string);

void pcf8574IntBacklight(uint8_t value);

#endif // PCF8574_H
