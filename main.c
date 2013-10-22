#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"

#define B_MENU	(1 << 7)
#define B_UP	(1 << 4)
#define B_DOWN	(1 << 5)
#define B_LEFT	(1 << 6)
#define B_RIGHT	(1 << 0)


#define B_ANY (B_MENU | B_UP | B_DOWN | B_LEFT | B_RIGHT)

int main(void)
{
	_delay_ms(100);
	gdInit();
	adcInit();

	sei();

	uint8_t *buf;
	while (1)
	{
		buf = getData();
		gdSpectrum(buf, MODE_STEREO);
	}
	return 0;
}
