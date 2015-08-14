#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>
#include "audio/audio.h"

/* Audio parameters values */
#define eepromVolume			0x00

/* FM tuner parameters valus */
#define eepromFMTuner			0x20
#define eepromFMStep			0x21
#define eepromFMMono			0x22
#define eepromFMCtrl			0x23
#define eepromFMFreq			0x24

/* Audio processor values */
#define eepromAudioproc			0x28
#define eepromInput				0x29
#define eepromLoudness			0x2A

/* Display parameters */
#define eepromSpMode			0x30
#define eepromDisplay			0x31
#define eepromBrStby			0x32
#define eepromBrWork			0x33
#define eepromFallSpeed			0x34

/* Other parameters */
#define eepromEncRes			0x38
#define eepromRC5Addr			0x39
#define eepromTempTH			0x3A
#define eepromAdcCorrL			0x3B
#define eepromAdcCorrR			0x3C
#define eepromExtFunc			0x3D

/* RC5 commands array */
#define eepromRC5Cmd			0x40

/* FM stations */
#define eepromStations			0x70
#define eepromFavStations		0xD8

/* Text labels (maximum 15 byte followed by \0) */
#define eepromLabelsAddr		0xF0

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
	LABEL_RC5_FALLSPEED,
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
	LABEL_SENSOR,

	LABEL_END
};

#endif /* EEPROM_H */
