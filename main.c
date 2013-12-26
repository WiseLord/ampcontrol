#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "eeprom.h"
#include "ks0108.h"
#include "fft.h"
#include "adc.h"
#include "input.h"
#include "i2c.h"
#include "param.h"
#include "ds1307.h"

void hwInit(void)	/* Hardware initialization */
{
	_delay_ms(100);
	gdInit();		/* Graphic display */
	rc5Init();		/* IR Remote control */
	adcInit();		/* Analog-to-digital converter */
	btnInit();		/* Buttons/encoder polling */
	I2CInit();		/* I2C bus */
	SMBF_DDR |= (STDBY | FAN | BCKL);
	SMBF_PORT &= ~(STDBY | MUTE | FAN | BCKL);
	sei();
	return;
}

int main(void)
{
	hwInit();

	uint8_t *buf;
	uint8_t command = CMD_NOCMD;
	uint8_t cmdCnt = 0;
	uint8_t i;

	displayMode mode = DISPLAY_TIME;
	uint8_t stdby = 1;

	muteSpeaker();

	while (1) {
		command = getCommand();
		cmdCnt = getCmdCount();

		if (!stdby && (command != CMD_NOCMD || getDisplayTime())) {
			/* Change current mode */
			switch (command) {
			case CMD_MENU:
				setDisplayTime(3000);
				switch (mode) {
				case DISPLAY_VOLUME:
					if (mode != DISPLAY_BASS)
						gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_BASS;
					break;
				case DISPLAY_BASS:
					if (mode != DISPLAY_MIDDLE)
						gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_MIDDLE;
					break;
				case DISPLAY_MIDDLE:
					if (mode != DISPLAY_TREBLE)
						gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_TREBLE;
					break;
				case DISPLAY_TREBLE:
					if (mode != DISPLAY_PREAMP)
						gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_PREAMP;
					break;
				case DISPLAY_PREAMP:
					if (mode != DISPLAY_BALANCE)
						gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_BALANCE;
					break;
				case DISPLAY_BALANCE:
				case DISPLAY_SPECTRUM:
				case DISPLAY_TIME:
					if (mode != DISPLAY_VOLUME)
						gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_VOLUME;
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
					gdFill(0x00, GD_CS1 | GD_CS2);
					mode = DISPLAY_VOLUME;
					break;
				default:
					break;
				}
				break;
			case CMD_TIME:
				setDisplayTime(3000);
				if (mode != DISPLAY_TIME)
					gdFill(0x00, GD_CS1 | GD_CS2);
				mode = DISPLAY_TIME;
				etm = EDIT_NOEDIT;
				break;
			case CMD_NUM1:
			case CMD_NUM2:
			case CMD_NUM3:
			case CMD_NUM4:
				setDisplayTime(3000);
				if (mode != DISPLAY_GAIN)
					gdFill(0x00, GD_CS1 | GD_CS2);
				mode = DISPLAY_GAIN;
				break;
			case CMD_STBY:
				SMBF_DDR &= ~MUTE;
				SMBF_PORT &= ~MUTE;
				_delay_ms(50);
				stdby = 1;
				SMBF_PORT &= ~STDBY;
				SMBF_PORT &= ~FAN;
				SMBF_PORT &= ~BCKL;
				gdFill(0x00, GD_CS1 | GD_CS2);
				mode = DISPLAY_TIME;
				muteSpeaker();
				saveParams();
			default:
				break;
			}

			/* Execute command */
			switch (command) {
			case CMD_VOL_UP:
				for (i = 0; i < cmdCnt; i++)
					switch (mode) {
					case DISPLAY_VOLUME:
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
					case DISPLAY_PREAMP:
						incVolume();
						break;
					case DISPLAY_GAIN:
						incGain(channel);
						break;
					case DISPLAY_BALANCE:
						incBalance();
						break;
					case DISPLAY_EDIT_TIME:
						setDisplayTime(30000);
						incTime();
						break;
					default:
						break;
					}
					break;
			case CMD_VOL_DOWN:
				for (i = 0; i < cmdCnt; i++)
					switch (mode) {
					case DISPLAY_VOLUME:
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
					case DISPLAY_PREAMP:
						decVolume();
						break;
					case DISPLAY_GAIN:
						decGain(channel);
						break;
					case DISPLAY_BALANCE:
						decBalance();
						break;
					case DISPLAY_EDIT_TIME:
						setDisplayTime(30000);
						decTime();
						break;
					default:
						break;
					}
					break;
			case CMD_SEARCH:
				setDisplayTime(3000);
				if (mode != DISPLAY_GAIN)
					gdFill(0x00, GD_CS1 | GD_CS2);
				else
					incChannel();
				mode = DISPLAY_GAIN;
				break;
			case CMD_STORE:
				setDisplayTime(30000);
				if (mode != DISPLAY_EDIT_TIME)
					gdFill(0x00, GD_CS1 | GD_CS2);
				mode = DISPLAY_EDIT_TIME;
				editTime();
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
			case CMD_DESCR:
				editSpMode();
				saveParams();
				break;
			default:
				break;
			}

			/* Show result */
			switch (mode) {
			case DISPLAY_VOLUME:
				showSpeaker(volumeLabel);
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
			case DISPLAY_PREAMP:
				showVolume(preampLabel);
				break;
			case DISPLAY_GAIN:
				showGain(channel, gainLabel);
				break;
			case DISPLAY_BALANCE:
				showBalance(balanceLabel);
				break;
			case DISPLAY_TIME:
			case DISPLAY_EDIT_TIME:
				showTime(0);
				break;
			default:
				break;
			}
		} else {
			if (!stdby) {
				if (mode != DISPLAY_SPECTRUM)
				{
					mode = DISPLAY_SPECTRUM;
					saveParams();
					etm = EDIT_NOEDIT;
				}
				buf = getData();
				gdSpectrum(buf, spMode);
			} else {
				setDisplayTime(2000);
				showTime(0);
				switch (command) {
				case CMD_STBY:
					stdby = 0;
					SMBF_PORT |= STDBY;
					_delay_ms(50);
					SMBF_DDR |= MUTE;
					SMBF_PORT |= MUTE;
					SMBF_PORT |= FAN;
					SMBF_PORT |= BCKL;
					loadParams();
					break;
				default:
					break;
				}
			}
		}
	}

	return 0;
}
