#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>
#include "audio/audio.h"

/* Audio parameters values */
#define EEPROM_VOLUME				0x00

/* Alarm */
#define EEPROM_ALARM_HOUR			0x10
#define EEPROM_ALARM_MIN			0x11
#define EEPROM_ALARM_MON			0x12

/* Audio bool parameters values */
#define EEPROM_LOUDNESS				0x1A
#define EEPROM_SURROUND				0x1B
#define EEPROM_EFFECT3D				0x1C
#define EEPROM_TONE_DEFEAT			0x1D

/* FM tuner parameters valus */
#define EEPROM_FM_TUNER				0x20
#define EEPROM_FM_STEP				0x21
#define EEPROM_FM_MONO				0x22
#define EEPROM_FM_CTRL				0x23
#define EEPROM_FM_FREQ				0x24

/* Audio processor values */
#define EEPROM_MAX_INPUT_CNT		0x26
#define EEPROM_AUDIOPROC			0x27
#define EEPROM_INPUT				0x28
#define EEPROM_INPUT_ICONS			0x29

/* Display parameters */
#define EEPROM_SP_MODE				0x30
#define EEPROM_DISPLAY				0x31
#define EEPROM_BR_STBY				0x32
#define EEPROM_BR_WORK				0x33
#define EEPROM_FALL_SPEED			0x34

/* Other parameters */
#define EEPROM_ENC_RES				0x36
#define EEPROM_RC_TYPE				0x38
#define EEPROM_RC_ADDR				0x39
#define EEPROM_TEMP_TH				0x3A
#define EEPROM_ADC_CORR_L			0x3B
#define EEPROM_ADC_CORR_R			0x3C
#define EEPROM_EXT_FUNC				0x3D
#define EEPROM_SILENCE_TIMER		0x3E

/* RC commands array */
#define EEPROM_RC_CMD				0x40

/* FM stations */
#define EEPROM_STATIONS				0x70
#define EEPROM_FAV_STATIONS			0xD8

/* Text labels (maximum 15 byte followed by \0) */
#define EEPROM_LABELS_ADDR			0xF0

#define EEPROM_SIZE					0x400

/* EEPROM saved labels */
enum {
	LABEL_MUTE = MODE_SND_END,
	LABEL_LOUDNESS,
	LABEL_SURROUND,
	LABEL_EFFECT_3D,
	LABEL_TONE_DEFEAT,
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

	LABEL_LEFT_CHANNEL,
	LABEL_RIGHT_CHANNEL,

	LABEL_TIMER,
	LABEL_WEEKDAYS,

	LABEL_END
};

#endif /* EEPROM_H */
