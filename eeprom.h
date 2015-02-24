#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>
#include "audio/audio.h"

/* Parameters values */
#define eepromVolume	((void*)0x00)
#define eepromBass		((void*)0x01)
#define eepromMiddle	((void*)0x02)
#define eepromTreble	((void*)0x03)
#define eepromPreamp	((void*)0x04)
#define eepromBalance	((void*)0x05)
#define eepromGain0		((void*)0x06)
#define eepromGain1		((void*)0x07)
#define eepromGain2		((void*)0x08)
#define eepromGain3		((void*)0x09)

/* Some values stored in EEPROM */
#define eepromLoudness	((void*)0x0C)
#define eepromInput		((void*)0x0D)
#define eepromAudioproc	((void*)0x0E)

#define eepromSpMode	((void*)0x10)
#define eepromDisplay	((void*)0x11)
#define eepromBrStby	((void*)0x12)
#define eepromBrWork	((void*)0x13)

#define eepromFMFreq	((void*)0x16)
#define eepromFMCtrl	((void*)0x18)
#define eepromFMMono	((void*)0x19)
#define eepromFMStep	((void*)0x1A)

#define eepromRC5Addr	((void*)0x1C)
#define eepromTempTH	((void*)0x1D)
#define eepromAdcCorrL	((void*)0x1E)
#define eepromAdcCorrR	((void*)0x1F)

/* RC5 commands array */
#define eepromRC5Cmd	((uint8_t*)0x40)

/* FM stations */
#define eepromStations	((uint16_t*)0x70)

/* Text labels (maximum 15 byte followed by \0) */
#define labelsAddr		((uint8_t*)0xF0)

#define EEPROM_SIZE				0x400

/* EEPROM saved labels */
enum {
	LABEL_MUTE = MODE_SND_END,
	LABEL_LOUDNESS,
	LABEL_ON,
	LABEL_OFF,
	LABEL_DB,

	LABEL_SUNDAY,
	LABEL_MONDAY,
	LABEL_TUESDAY,
	LABEL_WEDNESDAY,
	LABEL_THURSDAY,
	LABEL_FRIDAY,
	LABEL_SADURDAY,

	LABEL_BR_WORK,

	LABEL_IN_STATUS,
	LABEL_REMOTE,
	LABEL_BUTTONS,
	LABEL_LEARN_MODE,
	LABEL_BUTTON,
	LABEL_ADDRESS,
	LABEL_COMMAND,

	LABEL_RC5_STBY,
	LABEL_RC5_MUTE,
	LABEL_RC5_MENU,
	LABEL_RC5_VOL_UP,
	LABEL_RC5_VOL_DOWN,
	LABEL_RC5_INPUT_0,
	LABEL_RC5_INPUT_1,
	LABEL_RC5_INPUT_2,
	LABEL_RC5_INPUT_3,
	LABEL_RC5_NEXT_INPUT,
	LABEL_RC5_TIME,
	LABEL_RC5_BACKLIGHT,
	LABEL_RC5_SP_MODE,
	LABEL_RC5_DISPLAY,
	LABEL_RC5_TIMER,
	LABEL_RC5_ALARM,

	LABEL_RC5_CHAN_UP,
	LABEL_RC5_CHAN_DOWN,
	LABEL_RC5_FM_TUNE,
	LABEL_RC5_FM_MONO,
	LABEL_RC5_FM_STORE,
	LABEL_RC5_1,
	LABEL_RC5_2,
	LABEL_RC5_3,
	LABEL_RC5_4,
	LABEL_RC5_5,
	LABEL_RC5_6,
	LABEL_RC5_7,
	LABEL_RC5_8,
	LABEL_RC5_9,
	LABEL_RC5_0,

	LABEL_LEFT_CHANNEL,
	LABEL_RIGHT_CHANNEL,

	LABEL_THRESHOLD,
	LABEL_SENSOR
};

#define LABELS_COUNT			66

#endif /* EEPROM_H */
