#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

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

/* Parameters min/max/step values */
#define eepromMinimums	((void*)0x10)
#define eepromMaximums	((void*)0x20)
#define eepromSteps		((void*)0x30)

/*Some stored in EEPROM values */
#define eepromLoudness	((void*)0x0C)
#define eepromChannel	((void*)0x0D)

#define eepromSpMode	((void*)0x1C)
#define eepromBCKL		((void*)0x1D)

#define eepromFMFreq	((void*)0x2C)

#define eepromRC5Addr	((void*)0x3C)

/* RC5 commands array */
#define eepromRC5Cmd	((uint8_t*)0x40)

/* FM stations */
#define eepromStations	((uint16_t*)0x60)

/* Text labels (maximum 15 byte followed by \0) */
#define labelsAddr		((uint8_t*)0xE0)

#define EEPROM_SIZE				0x200
#define LABELS_COUNT			22

void loadLabels(uint8_t **txtLabels);

#endif /* EEPROM_H */
