#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "tda7439.h"
#include "i2c.h"

regParam *curParam;

#define SHOW_FFT	0
#define SHOW_PARAM	1
#define SHOW_TIME	2

int main(void)
{
	_delay_ms(100);
	gdInit();

	rc5Init();
	adcInit();
	btnInit();

	I2CInit();

/*
	DS1307Write(0x02, 0x19);
	DS1307Write(0x01, 0x21);
	DS1307Write(0x00, 0x00);
*/
	sei();

	uint8_t *buf;

	uint8_t command = 0xFF;
	uint16_t marker = 0;

	uint8_t mode = SHOW_FFT;

	loadParams();
	curParam = nextParam(0);

	while (1) {
		command = getCommand();

		if ((command != CMD_NOCMD) || marker) {
			if (marker == 0)
				gdFill(0x00, CS1 | CS2);

			switch (command) {
			case CMD_VOL_UP:
				incParam(curParam);
				marker = 200;
				mode = SHOW_PARAM;
				break;
			case CMD_VOL_DOWN:
				decParam(curParam);
				marker = 200;
				mode = SHOW_PARAM;
				break;
			case CMD_MENU:
				curParam = nextParam(curParam);
				marker = 200;
				mode = SHOW_PARAM;
				break;
			case CMD_TIMER:
				if (mode != SHOW_TIME)
					gdFill(0x00, CS1 | CS2);
				marker = 300;
				mode = SHOW_TIME;
				break;
			default:
				break;
			}

			if (mode == SHOW_PARAM)
				showParam(curParam);
			else if (mode == SHOW_TIME)
				showTime();

			if (--marker == 0) {
				saveParams();
				curParam = nextParam(0);
			}
		} else {
			buf = getData();
			gdSpectrum(buf, MODE_STEREO);
		}
	}

	return 0;
}
