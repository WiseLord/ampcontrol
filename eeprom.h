#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

/* Parameters values */
#define eepromVolume	((void*)0x00)
#define eepromBass		((void*)0x01)
#define eepromTreble	((void*)0x02)
#define eepromBalance	((void*)0x03)
#define eepromFrontRear	((void*)0x04)
#define eepromGain0		((void*)0x05)
#define eepromGain1		((void*)0x06)
#define eepromGain2		((void*)0x07)

/* Some values stored in EEPROM */
#define eepromLoudness	((void*)0x0C)
#define eepromChannel	((void*)0x0D)

#define eepromSpMode	((void*)0x10)
#define eepromDisplay	((void*)0x11)
#define eepromBrStby	((void*)0x12)
#define eepromBrWork	((void*)0x13)

#define eepromLang		((void*)0x18)

#define eepromRC5Addr	((void*)0x1C)
#define eepromTempTH	((void*)0x1D)

/* RC5 commands array */
#define eepromRC5Cmd	((uint8_t*)0x020)

/* Text labels (maximum 15 bytes followed by \0) */
#define labelsAddrEn	((uint8_t*)0x040)
#define labelsAddrRu	((uint8_t*)0x110)

#define EEPROM_SIZE		0x200

/* EEPROM saved labels */
enum {
	LABEL_VOLUME,
	LABEL_BASS,
	LABEL_TREBLE,
	LABEL_BALANCE,
	LABEL_FRONTREAR,
	LABEL_GAIN0,
	LABEL_GAIN1,
	LABEL_GAIN2,

	LABEL_MUTE,
	LABEL_LOUDNESS,
	LABEL_ON,
	LABEL_OFF,

	LABEL_SUNDAY,
	LABEL_MONDAY,
	LABEL_THUESDAY,
	LABEL_WEDNESDAY,
	LABEL_THURSDAY,
	LABEL_FRIDAY,
	LABEL_SADURDAY,

	LABEL_BR_WORK
};

#define LABELS_COUNT			20

#endif /* EEPROM_H */
