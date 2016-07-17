#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

/* Parameters values */
#define EEPROM_VOLUME			0x00
#define EEPROM_BASS				0x01
#define EEPROM_MIDDLE			0x02
#define EEPROM_TREBLE			0x03
#define EEPROM_PREAMP			0x04
#define EEPROM_BALANCE			0x05
#define EEPROM_GAIN0			0x06
#define EEPROM_GAIN1			0x07
#define EEPROM_GAIN2			0x08
#define EEPROM_GAIN3			0x09

/* Some values stored in EEPROM */
#define EEPROM_LOUDNESS			0x0C
#define EEPROM_INPUT			0x0D

#define EEPROM_SP_MODE			0x10
#define EEPROM_DISPLAY			0x11
#define EEPROM_BR_STBY			0x12
#define EEPROM_BR_WORK			0x13

#define EEPROM_FM_TUNER			0x1A
#define EEPROM_FM_MONO			0x19
#define EEPROM_FM_CTRL			0x18
#define EEPROM_FM_FREQ			0x16
#define EEPROM_FM_STEP1			0xD4
#define EEPROM_FM_STEP2			0xD5
#define EEPROM_FM_FREQ_MIN		0xEC
#define EEPROM_FM_FREQ_MAX		0xEE

#define EEPROM_RC_ADDR			0x1C
#define EEPROM_TEMP_TH			0x1D

/* RC5 commands array */
#define EEPROM_RC_CMD			0x40

/* FM stations */
#define EEPROM_STATIONS			0x70
#define EEPROM_FAV_STATIONS		0xD8

/* Text labels (maximum 15 byte followed by \0) */
#define EEPROM_LABELS_ADDR		0xF0

#define EEPROM_SIZE				0x200

/* EEPROM saved labels */
enum {
	LABEL_VOLUME,
	LABEL_BASS,
	LABEL_MIDDLE,
	LABEL_TREBLE,
	LABEL_PREAMP,
	LABEL_FRONTREAR,
	LABEL_BALANCE,
	LABEL_GAIN0,
	LABEL_GAIN1,
	LABEL_GAIN2,
	LABEL_GAIN3,
	LABEL_MUTE,
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

	LABEL_BR_WORK
};

#define LABELS_COUNT			24

#endif /* EEPROM_H */
