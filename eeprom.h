#ifndef EEPROM_H
#define EEPROM_H

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
#define eepromRC5Addr	((void*)0x40)
#define eepromSpMode	((void*)0x41)
#define eepromChanCnt	((void*)0x42)
#define eepromICSelect	((void*)0x43)
#define eepromBCKL		((void*)0x44)
#define eepromLoudness	((void*)0x45)
#define eepromChannel	((void*)0x46)

/* RC5 commands array */
#define eepromRC5Cmd	((uint8_t*)0x50)

/* Text labels (maximum 15 byte followed by \0) */
#define labelsAddr		((uint8_t*)0x070)

#endif // EEPROM_H
