#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "tda7439.h"

regParam *curParam;

int main(void)
{
	_delay_ms(100);
	gdInit();

	rc5Init();
	adcInit();
	btnInit();

	sei();

	uint8_t *buf;

	uint8_t command = 0;
	uint8_t marker = 0;

	loadParams();
	curParam = nextParam(0);

	while (1)
	{
		command = getBtnComm();

		if (command | marker)
		{
			if (marker == 0)
				gdFill(0x00, CS1 | CS2);

			switch (command) {
			case COMM_ENC_UP:
				incParam(curParam);
				marker = 200;
				break;
			case COMM_ENC_DOWN:
				decParam(curParam);
				marker = 200;
				break;
			case COMM_BTN_MENU:
				curParam = nextParam(curParam);
				marker = 200;
				break;
			default:
				break;
			}

			showParam(curParam);

			if (--marker == 0)
			{
				saveParams();
				curParam = nextParam(0);
			}
		}
		else
		{
			buf = getData();
			gdSpectrum(buf, MODE_STEREO);
		}
	}

	return 0;
}
