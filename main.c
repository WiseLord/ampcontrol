#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "tda7439.h"

#include "rc5.h"

#define RC5_STBT_MASK 0x3000
#define RC5_TOGB_MASK 0x0800
#define RC5_ADDR_MASK 0x007C
#define RC5_COMM_MASK 0x003F

#define RC5_ADDR 0x400

#define RC5_MENU 0x1B
#define RC5_VOL_UP 0x10
#define RC5_VOL_DOWN 0x11

regParam *curParam;

int main(void)
{
	_delay_ms(100);
	gdInit();

	rc5Init();

	/* Enable interrupts */
	sei();

//	for(;;)
//	{
//		uint16_t command;

//		/* Poll for new RC5 command */
//		if(rc5NewComm(&command))
//		{
//			/* Reset RC5 lib so the next command
//			* can be decoded. This is a must! */
//			rc5Reset();

//			/* Do something with the command
//			 * Perhaps validate the start bits and output
//			 * it via UART... */
////			if(RC5_GetStartBits(command) != 3)
////			{
////				/* ERROR */
////			}

//			gdSetPos(0, 0);
//			gdWriteNum(command, 6);
//		}
//	}



	adcInit();
	btnInit();

	sei();

	uint8_t *buf;
	int8_t delta;
	uint8_t btn, prevBtn = 0;
	uint8_t marker = 0;

	loadParams();
	curParam = nextParam(0);

	uint16_t command;

	while (1)
	{
		btn = getButtons();

		if(rc5NewComm(&command))
			rc5Reset();

		if ((btn == BTN_MENU) && (btn != prevBtn))
		{
			if (marker)
				curParam = nextParam(curParam);
		}
		prevBtn = btn;

		delta = getEncValue();

		if ((command & RC5_COMM_MASK) == RC5_VOL_UP)
		{
			command = 0;
			delta += 1;
		}
		if ((command & RC5_COMM_MASK) == RC5_VOL_DOWN)
		{
			command = 0;
			delta -= 1;
		}

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
