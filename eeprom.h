#ifndef EEPROM_H
#define EEPROM_H

/* Addresses in EEPROM with parameters */

#define eepromPreamp	((void*)0x00)
#define eepromBass		((void*)0x01)
#define eepromMiddle	((void*)0x02)
#define eepromTreble	((void*)0x03)
#define eepromBalance	((void*)0x04)
#define eepromVolume	((void*)0x05)
#define eepromChannel	((void*)0x06)
#define eepromGain0		((void*)0x07)
#define eepromGain1		((void*)0x08)
#define eepromGain2		((void*)0x09)
#define eepromGain3		((void*)0x0A)
#define eepromSpMode	((void*)0x0B)
#define eepromRC5Addr	((void*)0x0C)

/* Addresses in EEPROM with text labels (maximum 15 byte followed by \0 */

#define volumeLabel		((uint8_t*)0x20)
#define bassLabel		((uint8_t*)0x30)
#define middleLabel		((uint8_t*)0x40)
#define trebleLabel		((uint8_t*)0x50)
#define balanceLabel	((uint8_t*)0x60)
#define preampLabel		((uint8_t*)0x70)
#define gainLabel		((uint8_t*)0x80)
#define fanLabel		((uint8_t*)0x90)
#define dbLabel			((uint8_t*)0xA0)
#define weekdayLabel	((uint8_t*)0xB0)

#endif // EEPROM_H
