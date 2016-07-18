#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>
#include "audio/audio.h"

// Audio parameters values
#define EEPROM_VOLUME			0x00

#define EEPROM_SP_MODE			0x10
#define EEPROM_DISPLAY			0x11
#define EEPROM_BR_STBY			0x12
#define EEPROM_BR_WORK			0x13

// Audio bool parameters values
#define EEPROM_AUDIOPROC		0x16
#define EEPROM_MAX_INPUT_CNT	0x17
#define EEPROM_INPUT			0x18

#define EEPROM_LOUDNESS			0x19
#define EEPROM_SURROUND			0x1A
#define EEPROM_EFFECT3D			0x1B
#define EEPROM_TONE_DEFEAT		0x1C

// RC5 commands array
#define EEPROM_RC_TYPE			0x1E
#define EEPROM_RC_ADDR			0x1F
#define EEPROM_RC_CMD			0x20

/* FM tuner parameters valus */
#define EEPROM_FM_TUNER			0x50
#define EEPROM_FM_CTRL			(EEPROM_FM_TUNER + 0x01)
#define EEPROM_FM_STEP1			(EEPROM_FM_TUNER + 0x02)
#define EEPROM_FM_STEP2			(EEPROM_FM_TUNER + 0x03)
#define EEPROM_FM_FREQ_MIN		(EEPROM_FM_TUNER + 0x04)
#define EEPROM_FM_FREQ_MAX		(EEPROM_FM_TUNER + 0x06)
#define EEPROM_FM_FREQ			(EEPROM_FM_TUNER + 0x08)
#define EEPROM_FM_MONO			(EEPROM_FM_TUNER + 0x0A)
#define EEPROM_FM_RDS			(EEPROM_FM_TUNER + 0x0B)

/* FM stations */
#define EEPROM_FAV_STATIONS		(EEPROM_FM_TUNER + 0x10)
#define EEPROM_STATIONS			(EEPROM_FM_TUNER + 0x24)

// Text labels (maximum 15 byte followed by \0)
#define EEPROM_LABELS_ADDR		0xF0

#define EEPROM_SIZE				0x200

/* EEPROM saved labels */
enum {
	LABEL_MUTE = MODE_SND_END,
	LABEL_LOUDNESS,
	LABEL_ON,
	LABEL_OFF,
	LABEL_DB,

	LABEL_SUNDAY,
	LABEL_MONDAY,
	LABEL_THUESDAY,
	LABEL_WEDNESDAY,
	LABEL_THURSDAY,
	LABEL_FRIDAY,
	LABEL_SADURDAY,

	LABEL_BR_WORK,

	LABEL_END
};

#endif /* EEPROM_H */
