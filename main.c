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
	adcInit();
	btnInit();

	sei();

	uint8_t *buf;
	int8_t delta;
	uint8_t btn, prevBtn = 0;
	uint8_t marker = 0;

	loadParams();
	curParam = nextParam(0);

	while (1)
	{
		btn = getButtons();
		if ((btn == BTN_MENU) && (btn != prevBtn))
		{
			if (marker)
				curParam = nextParam(curParam);
		}
		prevBtn = btn;

		delta = getEncValue();
		if ((btn == BTN_MENU) | delta | marker)
		{
			if (btn | delta)
			{
				if (!marker)
					gdFill(0x00, CS1 | CS2);
				marker = 200;
			}
			changeParam(curParam, delta);

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
	gdFill(0x00, CS1 | CS2);
	return 0;
}
