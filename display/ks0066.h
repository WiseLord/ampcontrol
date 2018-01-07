#ifndef KS0066_H
#define KS0066_H

#include <inttypes.h>

/* Display size */
#ifdef _KS0066_16X2_PCF8574
#define KS0066_SCREEN_WIDTH			16
#define KS0066_SCREEN_HEIGHT		2
#define KS0066_WIRE_PCF8574
#endif
#ifdef _KS0066_16X2
#define KS0066_SCREEN_WIDTH			16
#define KS0066_SCREEN_HEIGHT		2
#define KS0066_WIRE_8BIT
#endif

/* KS0066 wiring type, if not set any, autoselect KS0066_WIRE_8BIT */
#ifndef KS0066_WIRE_8BIT
#ifndef KS0066_WIRE_4BIT
#ifndef KS0066_WIRE_PCF8574
#define KS0066_WIRE_4BIT
#endif
#endif
#endif

#if defined(KS0066_WIRE_PCF8574)
#include "../i2c.h"
/* PCF8574 expander address inputs - 0 if soldered to ground, 1 if not */
#define PCF8574_A2					(0 << 3)
#define PCF8574_A1					(0 << 2)
#define PCF8574_A0					(0 << 1)
#define PCF8574_ADDR				(0x40 | PCF8574_A2 | PCF8574_A1 | PCF8574_A0)
#define PCF8574_BL_LINE				(1<<3)
#define PCF8574_E_LINE				(1<<2)
#define PCF8574_RW_LINE				(1<<1)
#define PCF8574_RS_LINE				(1<<0)
#endif

/* KS0066 instruction registers */
#define KS0066_NO_COMMAND			0x00

#define KS0066_CLEAR				0x01
#define KS0066_HOME					0x02

#define KS0066_SET_MODE				0x04
	#define KS0066_INC_ADDR			0x02
	#define KS0066_SHIFT_WINDOW		0x01

#define KS0066_DISPLAY				0x08
	#define KS0066_DISPAY_ON		0x04
	#define KS0066_CURSOR_ON		0x02
	#define KS0066_CUR_BLINK_ON		0x01

#define KS0066_SHIFT				0x10
	#define KS0066_SHIFT_DISPLAY	0x08
	#define KS0066_SHIFT_RIGHT		0x04

#define KS0066_FUNCTION				0x20
	#define KS0066_8BIT				0x10
	#define KS0066_2LINES			0x08
	#define KS0066_5X10				0x04

#define KS0066_SET_CGRAM			0x40

#define KS0066_SET_DDRAM			0x80

/* Some useful defines */
#define KS0066_LINE_WIDTH			0x40
#define KS0066_STA_BUSY				0x80

#define KS0066_BCKL_OFF				0
#define KS0066_BCKL_ON				1

#define KS0066_MIN_BRIGHTNESS		0
#define KS0066_MAX_BRIGHTNESS		32

#define KS0066_DATA_CGRAM			0
#define KS0066_DATA_DDRAM			1

/* Functions */
void ks0066Clear(void);
void ks0066Init(void);

void ks0066StartSym(uint8_t addr);
void ks0066SetXY(uint8_t x, uint8_t y);

void ks0066WriteData(uint8_t data);
void ks0066WriteString(char *string);
void ks0066WriteTail(uint8_t ch, uint8_t pos);

void pcf8574SetBacklight(uint8_t value);
void ks0066SetBrightness(uint8_t br);

#endif /* KS0066_H */
