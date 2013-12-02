#include <util/delay.h>
#include <avr/interrupt.h>

#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "i2c.h"

#include "param.h"
uint8_t volumeLabel[] = "Усиление";
uint8_t bassLabel[] = "Тембр НЧ";
uint8_t middleLabel[] = "Тембр СЧ";
uint8_t trebleLabel[] = "Тембр ВЧ";
uint8_t balanceLabel[] = "Баланс";
uint8_t speakerLabel[] = "Громкость";
uint8_t gainLabel[] = "Канал";

void hwInit(void)	/* Hardware initialization */
{
	_delay_ms(100);
	gdInit();		/* Graphic display */
	rc5Init();		/* IR Remote control */
	adcInit();		/* Analog-to-digital converter */
	btnInit();		/* Buttons/encoder polling */
	I2CInit();		/* I2C bus */
	sei();
	return;
}

int main(void)
{
	hwInit();
/*
	DS1307Write(0x00, 0x00);
	DS1307Write(0x01, 0x37);
	DS1307Write(0x02, 0x19);

	DS1307Write(0x03, 0x01);

	DS1307Write(0x04, 0x01);
	DS1307Write(0x05, 0x12);
	DS1307Write(0x06, 0x13);
*/

	uint8_t *buf;
	uint8_t command = 0xFF;
	displayMode mode = DISPLAY_SPECTRUM;

	loadParams();

	while (1) {
		command = getCommand();

		if (command != CMD_NOCMD || getDisplayTime()) {
			/* Change current mode */
			switch (command) {
			case CMD_MENU:
				setDisplayTime(3000);
				switch (mode) {
				case DISPLAY_SPEAKER:
					if (mode != DISPLAY_BASS)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_BASS;
					break;
				case DISPLAY_BASS:
					if (mode != DISPLAY_MIDDLE)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_MIDDLE;
					break;
				case DISPLAY_MIDDLE:
					if (mode != DISPLAY_TREBLE)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_TREBLE;
					break;
				case DISPLAY_TREBLE:
					if (mode != DISPLAY_VOLUME)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_VOLUME;
					break;
				case DISPLAY_VOLUME:
					if (mode != DISPLAY_GAIN)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_GAIN;
					break;
				case DISPLAY_GAIN:
					if (mode != DISPLAY_BALANCE)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_BALANCE;
					break;
				case DISPLAY_BALANCE:
				case DISPLAY_SPECTRUM:
				case DISPLAY_TIME:
					if (mode != DISPLAY_SPEAKER)
						gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_SPEAKER;
					break;
				default:
					break;
				}
				break;
			case CMD_VOL_UP:
			case CMD_VOL_DOWN:
				setDisplayTime(2000);
				switch (mode) {
				case DISPLAY_SPECTRUM:
				case DISPLAY_TIME:
					gdFill(0x00, CS1 | CS2);
					mode = DISPLAY_SPEAKER;
					break;
				default:
					break;
				}
				break;
			case CMD_TIME:
				setDisplayTime(3000);
				if (mode != DISPLAY_TIME)
					gdFill(0x00, CS1 | CS2);
				mode = DISPLAY_TIME;
				break;
			case CMD_SEARCH:
			case CMD_NUM1:
			case CMD_NUM2:
			case CMD_NUM3:
			case CMD_NUM4:
				setDisplayTime(3000);
				if (mode != DISPLAY_GAIN)
					gdFill(0x00, CS1 | CS2);
				mode = DISPLAY_GAIN;
				break;
			default:
				break;
			}

			/* Execute command */
			switch (command) {
			case CMD_VOL_UP:
				switch (mode) {
				case DISPLAY_SPEAKER:
					incSpeaker();
					break;
				case DISPLAY_BASS:
					incBMT(&bass);
					break;
				case DISPLAY_MIDDLE:
					incBMT(&middle);
					break;
				case DISPLAY_TREBLE:
					incBMT(&treble);
					break;
				case DISPLAY_VOLUME:
					incVolume();
					break;
				case DISPLAY_GAIN:
					incGain(channel);
					break;
				case DISPLAY_BALANCE:
					incBalance();
					break;
				default:
					break;
				}
				break;
			case CMD_VOL_DOWN:
				switch (mode) {
				case DISPLAY_SPEAKER:
					decSpeaker();
					break;
				case DISPLAY_BASS:
					decBMT(&bass);
					break;
				case DISPLAY_MIDDLE:
					decBMT(&middle);
					break;
				case DISPLAY_TREBLE:
					decBMT(&treble);
					break;
				case DISPLAY_VOLUME:
					decVolume();
					break;
				case DISPLAY_GAIN:
					decGain(channel);
					break;
				case DISPLAY_BALANCE:
					decBalance();
					break;
				default:
					break;
				}
				break;
			case CMD_SEARCH:
				incChannel();
				break;
			case CMD_NUM1:
				setChannel(0);
				break;
			case CMD_NUM2:
				setChannel(1);
				break;
			case CMD_NUM3:
				setChannel(2);
				break;
			case CMD_NUM4:
				setChannel(3);
				break;
			default:
				break;
			}

			/* Show result */
			switch (mode) {
			case DISPLAY_SPEAKER:
				showSpeaker(speakerLabel);
				break;
			case DISPLAY_BASS:
				showBMT(&bass, bassLabel);
				break;
			case DISPLAY_MIDDLE:
				showBMT(&middle, middleLabel);
				break;
			case DISPLAY_TREBLE:
				showBMT(&treble, trebleLabel);
				break;
			case DISPLAY_VOLUME:
				showVolume(volumeLabel);
				break;
			case DISPLAY_GAIN:
				showGain(channel, gainLabel);
				break;
			case DISPLAY_BALANCE:
				showBalance(balanceLabel);
				break;
			case DISPLAY_TIME:
				showTime();
				break;
			default:
				break;
			}
		} else {
			if (mode != DISPLAY_SPECTRUM)
			{
				mode = DISPLAY_SPECTRUM;
				saveParams();
			}
			buf = getData();
			gdSpectrum(buf, MODE_STEREO);
		}
	}

	return 0;
}
