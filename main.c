#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"


int main(void)
{
	_delay_ms(100);
	gdInit();
	adcInit();
	btnInit();

	sei();

	int vol = 200;

	int8_t cnt;

	uint8_t *buf;

	while (1)
	{
		cnt = getEncValue();
		if (cnt)
			vol += cnt;
		else
		{
			buf = getData();
			gdSpectrum(buf, MODE_STEREO);
//			gdSetPos(96, 0);
//			gdWriteNum(vol, 5);
		}
	}
	return 0;
}
